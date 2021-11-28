#include "server.h"

#include <QTimer>

#include <version_string.h>

#include "commandContainer.pb.h"
#include "lobbyCommand.pb.h"
#include "serverMessage.pb.h"
#include "sessionEvent.pb.h"

#include "globalAbilities/globalAbilities.h"
#include "cardDatabase.h"

#include <QDebug>

Server::Server(std::unique_ptr<ConnectionManager> cm)
    : mConnectionManager(std::move(cm)), mNextGameId(0) {
    qRegisterMetaType<std::shared_ptr<CommandContainer>>("std::shared_ptr<CommandContainer>");
    qRegisterMetaType<std::shared_ptr<ServerMessage>>("std::shared_ptr<ServerMessage>");

    decodeGlobalAbilities();

    mConnectionManager->initialize(this);

    if (mConnectionManager->isLocal())
        return;

    mPingClock = new QTimer(this);
    connect(mPingClock, SIGNAL(timeout()), this, SIGNAL(pingClockTimeout()));
    mPingClock->start(mClientKeepalive  * 1000);

    mNotifyClock = new QTimer(this);
    connect(mNotifyClock, SIGNAL(timeout()), this, SLOT(sendLobbyInfo()));
    mNotifyClock->start(mSubscribersNotifyInterval * 1000);
}

EventLobbyInfo Server::lobbyInfo() {
    int userCount = connectedUsersCount();
    QReadLocker locker(&mPlayQueueLock);
    EventLobbyInfo info;
    info.set_user_count(userCount);
    for (const auto& [_, user]: mPlayQueue) {
        auto userInfo = info.add_user_info();
        userInfo->set_id(user->id());
        userInfo->set_name(user->name());
    }
    return info;
}

int Server::connectedUsersCount() {
    QReadLocker locker(&mClientsLock);
    return static_cast<int>(mClients.size());
}

int Server::nextGameId() {
    return ++mNextGameId;
}

void Server::sendLobbyInfo() {
    QReadLocker locker(&mSubscribersLock);
    for (auto client: mLobbySubscribers) {
        client->sendLobbyEvent(lobbyInfo());
    }
}

ServerGame* Server::game(int id) {
    if (!mGames.count(id))
        return nullptr;

    return mGames.at(id).get();
}

ServerProtocolHandler* Server::addClient(std::unique_ptr<ServerProtocolHandler> client) {
    QWriteLocker locker(&mClientsLock);
    static int clientId = 0;
    int newId = ++clientId;
    client->setId(newId);
    return mClients.emplace(newId, std::move(client)).first->second.get();
}

void Server::removeClient(ServerProtocolHandler *client) {
    QWriteLocker locker(&mClientsLock);
    removeClientFromPlayQueue(client);
    removeLobbySubscriber(client);
    client->refuseAllInvites();
    if (!mClients.contains(client->id()))
        return;

    mClients.at(client->id()).release();
    mClients.erase(client->id());
}

ServerGame* Server::createGame(const CommandCreateGame &cmd, ServerProtocolHandler *client) {
    QWriteLocker locker(&mGamesLock);
    int newGameId = nextGameId();
    auto newGame = mGames.emplace(newGameId, std::make_unique<ServerGame>(newGameId, cmd.description())).first->second.get();
    locker.unlock();

    QReadLocker readLocker(&mGamesLock);
    newGame->addPlayer(client);
    return newGame;
}

void Server::createGame(ServerProtocolHandler *client1, ServerProtocolHandler *client2) {
    QWriteLocker locker(&mGamesLock);
    int newGameId = nextGameId();
    auto newGame = mGames.emplace(newGameId, std::make_unique<ServerGame>(newGameId, "")).first->second.get();
    locker.unlock();

    QReadLocker readLocker(&mGamesLock);
    newGame->addPlayer(client1);
    newGame->addPlayer(client2);
}

void Server::removeGame(int id) {
    QWriteLocker locker(&mGamesLock);

    if (!mGames.contains(id))
        return;

    auto &game = mGames.at(id);
    QMutexLocker gameLocker(&game->mGameMutex);
    game->close();
    gameLocker.unlock();
    mGames.erase(id);
}

void Server::processGameJoinRequest(const CommandJoinGame &cmd, ServerProtocolHandler *client) {
    QReadLocker locker(&mGamesLock);
    auto g = game(cmd.game_id());
    if (!g)
        return;
    g->addPlayer(client);
}

int Server::maxClientInactivityTime() const {
    return mConnectionManager->isLocal() ? 999999 : 15;
}

void Server::sendServerIdentification(ServerProtocolHandler *client) {
    EventServerHandshake event;
    event.set_version(VERSION_STRING);
    event.set_database_version(CardDatabase::get().version());
    event.set_client_id(client->id());
    client->sendSessionEvent(event);
}

void Server::sendDatabase(ServerProtocolHandler *client) {
    auto data = CardDatabase::get().fileData();
    EventDatabase event;
    event.set_database(data);
    client->sendSessionEvent(event);
}

void Server::addLobbySubscriber(ServerProtocolHandler *client) {
    QWriteLocker locker(&mSubscribersLock);
    mLobbySubscribers.emplace(client);
    locker.unlock();

    client->sendLobbyEvent(lobbyInfo());
}

void Server::removeLobbySubscriber(ServerProtocolHandler *client) {
    QWriteLocker locker(&mSubscribersLock);
    mLobbySubscribers.erase(client);
}

void Server::inviteToPlay(ServerProtocolHandler *client, const CommandInviteToPlay &cmd) {
    QWriteLocker locker(&mPlayQueueLock);
    if (client->hasOutcomingInvite() || client->invited())
        return;

    if (!mPlayQueue.contains(cmd.user_id())) {
        locker.unlock();
        EventInviteDeclined event;
        event.set_reason(InviteRefusalReason::LeftQueue);
        client->sendLobbyEvent(event);
        client->sendLobbyEvent(lobbyInfo());
        return;
    }
    mPlayQueue.erase(client->id());
    mPlayQueue.at(cmd.user_id())->sendInvite(client);
    client->setOutcomingInvite(true, cmd.user_id());
    client->sendLobbyEvent(EventInviteSent());
}

void Server::refuseInviteUnsafe(int inviterId, InviteRefusalReason reason) {
    if (!mClients.contains(inviterId))
        return;

    EventInviteDeclined event;
    event.set_reason(reason);
    mClients.at(inviterId)->sendLobbyEvent(event);
}

void Server::refuseInvite(int inviterId, InviteRefusalReason reason) {
    QReadLocker locker(&mClientsLock);
    refuseInviteUnsafe(inviterId, reason);
}

void Server::cancelInvite(ServerProtocolHandler *client) {
    QReadLocker clientsLocker(&mClientsLock);
    QWriteLocker locker(&mPlayQueueLock);
    if (!client->hasOutcomingInvite())
        return;

    int inviteeId = client->inviteeId();
    client->setOutcomingInvite(false);
    if (!mClients.contains(inviteeId))
        return;
    mClients.at(inviteeId)->inviteWithdrawn(client);

    if (client->inQueue())
        mPlayQueue.emplace(client->id(), client);
}

void Server::declineInvite(ServerProtocolHandler *client, const CommandDeclineInvite &cmd) {
    QReadLocker clientsLocker(&mClientsLock);
    QWriteLocker locker(&mPlayQueueLock);
    if (!client->removeInvite(cmd.inviter_id()))
        return;
    if (!mClients.contains(cmd.inviter_id()))
        return;
    auto inviter = mClients.at(cmd.inviter_id()).get();
    inviter->inviteDeclined(cmd);
    if (inviter->inQueue())
        mPlayQueue.emplace(inviter->id(), inviter);
}

void Server::acceptInvite(ServerProtocolHandler *invitee, const CommandAcceptInvite &cmd) {
    QReadLocker clientsLocker(&mClientsLock);
    QWriteLocker locker(&mPlayQueueLock);

    if (!invitee->removeInvite(cmd.inviter_id()))
        return;
    if (!mClients.contains(cmd.inviter_id()))
        return;
    auto inviter = mClients.at(cmd.inviter_id()).get();
    if (!inviter->hasOutcomingInvite() || inviter->inviteeId() != invitee->id()) {
        return;
    }
    inviter->inviteAccepted();
    invitee->refuseAllInvites();
    invitee->setInQueue(false);
    mPlayQueue.erase(invitee->id());
    locker.unlock();

    createGame(inviter, invitee);
}

void Server::addClientToPlayQueue(ServerProtocolHandler *client) {
    QWriteLocker locker(&mPlayQueueLock);
    if (mPlayQueue.contains(client->id()))
        return;
    mPlayQueue.emplace(client->id(), client);
    client->setInQueue(true);
    locker.unlock();
    client->sendLobbyEvent(lobbyInfo());
}

void Server::removeClientFromPlayQueue(ServerProtocolHandler *client) {
    QWriteLocker locker(&mPlayQueueLock);
    client->setInQueue(false);
    mPlayQueue.erase(client->id());
}
