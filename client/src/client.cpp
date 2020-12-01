#include "client.h"

#include "gameCommand.pb.h"
#include "lobbyCommand.pb.h"
#include "lobbyEvent.pb.h"
#include "serverMessage.pb.h"

namespace {
std::shared_ptr<CommandContainer> prepareLobbyCommand(const ::google::protobuf::Message &cmd) {
    LobbyCommand lobbyCmd;
    lobbyCmd.mutable_command()->PackFrom(cmd);

    auto cont = std::make_shared<CommandContainer>();
    cont->mutable_command()->PackFrom(lobbyCmd);
    return cont;
}

std::shared_ptr<CommandContainer> prepareGameCommand(const ::google::protobuf::Message &cmd) {
    GameCommand gameCmd;
    gameCmd.mutable_command()->PackFrom(cmd);

    auto cont = std::make_shared<CommandContainer>();
    cont->mutable_command()->PackFrom(gameCmd);
    return cont;
}
}

Client::Client(std::unique_ptr<ClientConnection> &&connection)
    : mConnection(std::move(connection)) {
    connect(this, SIGNAL(queueCommand(std::shared_ptr<CommandContainer>)),
        this, SLOT(sendCommandContainer(std::shared_ptr<CommandContainer>)));
    connect(mConnection.get(), SIGNAL(messageReady(std::shared_ptr<ServerMessage>)),
            this, SLOT(processServerMessage(std::shared_ptr<ServerMessage>)));
}

void Client::sendLobbyCommand(const ::google::protobuf::Message &cmd) {
    sendCommand(prepareLobbyCommand(cmd));
}

void Client::sendGameCommand(const ::google::protobuf::Message &cmd) {
    sendCommand(prepareGameCommand(cmd));
}

void Client::processServerMessage(std::shared_ptr<ServerMessage> message) {
    if (message->message().Is<LobbyEvent>()) {
        LobbyEvent event;
        message->message().UnpackTo(&event);
        processLobbyEvent(event);
    }
}

void Client::processLobbyEvent(LobbyEvent &event) {
    if (event.event().Is<EventGameJoined>()) {
        auto ev = std::make_shared<EventGameJoined>();
        event.event().UnpackTo(ev.get());
        emit gameJoinedEventReceived(ev);
    }
}
