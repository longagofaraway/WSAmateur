#include "lobby.h"

#include "lobbyCommand.pb.h"
#include "lobbyEvent.pb.h"

Lobby::Lobby() {

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

void Lobby::joinQueue() {
    client->sendLobbyCommand(CommandEnterQueue());
}

bool Lobby::canInvite(int row) {
    return client->id() != model.idByRow(row);
}
