#include "lobby.h"

#include "lobbyCommand.pb.h"
#include "lobbyEvent.pb.h"

Lobby::Lobby() {

}

Lobby::~Lobby() {
    leaveQueue();
    disconnect(client, &Client::lobbyEventReceived, this, &Lobby::processLobbyEvent);
}

void Lobby::init(Client *client_) {
    client = client_;

    connect(client, &Client::lobbyEventReceived, this, &Lobby::processLobbyEvent);
    client->sendLobbyCommand(CommandEnterLobby());
}

void Lobby::componentComplete() {
    QQuickItem::componentComplete();

    emit lobbyCreated();
}

void Lobby::processLobbyEvent(const std::shared_ptr<LobbyEvent> event) {
    if (event->event().Is<EventLobbyInfo>()) {
        EventLobbyInfo ev;
        event->event().UnpackTo(&ev);
        lobbyInfoReceived(ev);
    } else if (event->event().Is<EventInviteSent>()) {
        emit inviteSent();
    } else if (event->event().Is<EventInvitedToPlay>()) {
        EventInvitedToPlay ev;
        event->event().UnpackTo(&ev);
        playerInviteReceived(ev);
    } else if (event->event().Is<EventInviteWithdrawn>()) {
        EventInviteWithdrawn ev;
        event->event().UnpackTo(&ev);
        playerInviteWithdrawn(ev);
    } else if (event->event().Is<EventInviteDeclined>()) {
        EventInviteDeclined ev;
        event->event().UnpackTo(&ev);
        playerInviteDeclined(ev);
    }
}

void Lobby::lobbyInfoReceived(EventLobbyInfo &event) {
    auto repeatedUserInfo = event .mutable_user_info();
    std::vector<UserInfo> userInfo(repeatedUserInfo->begin(), repeatedUserInfo->end());
    model.update(std::move(userInfo));
    emit userCountChanged(event.user_count());
    return;
    // for testing purposes
    static bool gameStarted = false;
    if (gameStarted)
        return;
    gameStarted = true;

    if (!event.user_info_size()) {
        client->sendLobbyCommand(CommandEnterQueue());
        return;
    }

    CommandInviteToPlay cmd;
    cmd.set_user_id(event.user_info(0).id());
    client->sendLobbyCommand(cmd);
}

void Lobby::playerInviteReceived(const EventInvitedToPlay &event) {
    inviteQueue.push_back(event.user_info());
    if (!isInvited) {
        isInvited = true;
        auto &name = inviteQueue.front().name();
        emit inviteReceived(QString::fromStdString(name));
    }
}

void Lobby::playerInviteWithdrawn(const EventInviteWithdrawn &event) {
    const auto &inviter = event.user_info();
    if (inviteQueue.empty())
        return;

    const auto &inviterOnScreen = inviteQueue.front();
    if (inviterOnScreen.id() == inviter.id()) {
        inviteQueue.pop_front();
        if (isInvited) {
            emit inviteWithdrawn();
        }
        if (inviteQueue.empty()) {
            isInvited = false;
        } else {
            auto &name = inviteQueue.front().name();
            emit inviteReceived(QString::fromStdString(name));
        }
    } else {
        auto it = std::remove_if(std::next(inviteQueue.begin()), inviteQueue.end(),
                                 [&inviter](const UserInfo &info) {
            return info.id() == inviter.id();
        });
        inviteQueue.erase(it, inviteQueue.end());
    }
}

void Lobby::playerInviteDeclined(const EventInviteDeclined &event) {
    emit inviteDeclined();
}

void Lobby::joinQueue() {
    client->sendLobbyCommand(CommandEnterQueue());
}

void Lobby::leaveQueue() {
    client->sendLobbyCommand(CommandLeaveQueue());
}

bool Lobby::canInvite(int row) {
    return client->id() != model.idByRow(row);
}

void Lobby::sendInvite() {
    auto id = model.selectedId();
    if (!id)
        return;

    CommandInviteToPlay cmd;
    cmd.set_user_id(id.value());
    client->sendLobbyCommand(cmd);
}

void Lobby::cancelInvite() {
    client->sendLobbyCommand(CommandCancelInvite());
}

void Lobby::refuseInvite() {
    CommandDeclineInvite cmd;
    auto &inviter = inviteQueue.front();
    cmd.set_inviter_id(inviter.id());
    cmd.set_reason(InviteRefusalReason::Declined);
    client->sendLobbyCommand(cmd);

    inviteQueue.pop_front();
    if (inviteQueue.empty()) {
        isInvited = false;
    } else {
        auto &name = inviteQueue.front().name();
        emit inviteReceived(QString::fromStdString(name));
    }
}

void Lobby::acceptInvite() {
    auto inviter = inviteQueue.front();
    inviteQueue.pop_front();

    while (inviteQueue.size()) {
        CommandDeclineInvite cmd;
        auto &ignored_inviter = inviteQueue.front();
        cmd.set_inviter_id(ignored_inviter.id());
        cmd.set_reason(InviteRefusalReason::LeftQueue);
        client->sendLobbyCommand(cmd);
        inviteQueue.pop_front();
    }

    CommandAcceptInvite cmd;
    cmd.set_inviter_id(inviter.id());
    client->sendLobbyCommand(cmd);
}
