#include "client.h"

#include "gameCommand.pb.h"
#include "gameEvent.pb.h"
#include "lobbyCommand.pb.h"
#include "lobbyEvent.pb.h"
#include "moveEvents.pb.h"
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

Client::Client(std::unique_ptr<ClientConnection> &&connection) {
    connection->setParent(this);
    mConnection = connection.release();
    connect(this, &Client::queueCommand, this, &Client::sendCommandContainer);
    connect(this, &Client::sigConnectToHost, this, &Client::doConnectToHost);

    connect(mConnection, &ClientConnection::messageReady, this, &Client::processServerMessage);
    connect(mConnection, &ClientConnection::connectionClosed, this, &Client::connectionClosed);

}

void Client::sendLobbyCommand(const ::google::protobuf::Message &cmd) {
    sendCommand(prepareLobbyCommand(cmd));
}

void Client::sendGameCommand(const ::google::protobuf::Message &cmd) {
    sendCommand(prepareGameCommand(cmd));
}

void Client::connectToHost(const std::string &hostname, uint16_t port) {
    auto host = QString::fromStdString(hostname);
    emit sigConnectToHost(host, port);
}

void Client::sendCommandContainer(std::shared_ptr<CommandContainer> command) {
    mConnection->sendMessage(command);
}

void Client::doConnectToHost(const QString &hostname, uint16_t port) {
    mConnection->connectToHost(hostname, port);
}

void Client::processServerMessage(std::shared_ptr<ServerMessage> message) {
    try {
    if (message->message().Is<LobbyEvent>()) {
        LobbyEvent event;
        message->message().UnpackTo(&event);
        processLobbyEvent(event);
    } else if (message->message().Is<GameEvent>()) {
        auto event = std::make_shared<GameEvent>();
        message->message().UnpackTo(event.get());
        emit gameEventReceived(event);
    }
    } catch(const std::exception &) {}
}

void Client::processLobbyEvent(const LobbyEvent &event) {
    if (event.event().Is<EventGameJoined>()) {
        auto ev = std::make_shared<EventGameJoined>();
        event.event().UnpackTo(ev.get());
        emit gameJoinedEventReceived(ev);
    } else if (event.event().Is<EventGameList>()) {
        auto ev = std::make_shared<EventGameList>();
        event.event().UnpackTo(ev.get());
        emit gameListReceived(ev);
    }
}
