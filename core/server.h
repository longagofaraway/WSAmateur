#pragma once

#include <unordered_map>
#include <unordered_set>

#include <QObject>
#include <QMutex>
#include <QReadWriteLock>

#include "lobbyEvent.pb.h"

#include "connectionManager.h"
#include "serverGame.h"
#include "serverProtocolHandler.h"
#include "serverUser.h"

class QTimer;
class CommandCreateGame;
class CommandJoinGame;
class CommandInviteToPlay;
class CommandDeclineInvite;

class Server : public QObject
{
    Q_OBJECT
protected:
    std::unique_ptr<ConnectionManager> mConnectionManager;
    std::unordered_map<int, std::unique_ptr<ServerGame>> mGames;
    std::unordered_map<int, std::unique_ptr<ServerProtocolHandler>> mClients;

    int mNextGameId;
    QReadWriteLock mClientsLock;

    QTimer *mPingClock = nullptr;
    const int mClientKeepalive = 3;

    std::unordered_set<ServerProtocolHandler*> mLobbySubscribers;
    QReadWriteLock mSubscribersLock;
    QTimer *mNotifyClock = nullptr;
    int mSubscribersNotifyInterval = 5;

    std::unordered_map<int, ServerProtocolHandler*> mPlayQueue;
    QReadWriteLock mPlayQueueLock;

public:
    Server(std::unique_ptr<ConnectionManager> cm);

    QReadWriteLock mGamesLock;
    EventLobbyInfo lobbyInfo();
    int connectedUsersCount();
    ServerGame* game(int id);

    ServerProtocolHandler* addClient(std::unique_ptr<ServerProtocolHandler>);
    void removeClient(ServerProtocolHandler *client);

    ServerGame* createGame(const CommandCreateGame &cmd, ServerProtocolHandler *client);
    void createGame(ServerProtocolHandler *client1, ServerProtocolHandler *client2);
    void removeGame(int id);
    void processGameJoinRequest(const CommandJoinGame &cmd, ServerProtocolHandler *client);

    int maxClientInactivityTime() const;
    void sendServerIdentification(ServerProtocolHandler *client);
    void sendDatabase(ServerProtocolHandler *client);
    void sendImageLinks(ServerProtocolHandler *client);
    void addLobbySubscriber(ServerProtocolHandler *client);
    void addClientToPlayQueue(ServerProtocolHandler *client);
    void removeClientFromPlayQueue(ServerProtocolHandler *client);
    void removeLobbySubscriber(ServerProtocolHandler *client);

    void inviteToPlay(ServerProtocolHandler *client, const CommandInviteToPlay& cmd);
    void refuseInviteUnsafe(int inviterId, InviteRefusalReason reason);
    void refuseInvite(int inviterId, InviteRefusalReason reason);
    void cancelInvite(ServerProtocolHandler *client);
    void declineInvite(ServerProtocolHandler *client, const CommandDeclineInvite &cmd);
    void acceptInvite(ServerProtocolHandler *invitee, const CommandAcceptInvite &cmd);

signals:
    void pingClockTimeout();

protected:
    int nextGameId();

private slots:
    void sendLobbyInfo();
};
