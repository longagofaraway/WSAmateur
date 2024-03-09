#include "client.h"

#include "gameCommand.pb.h"
#include "gameEvent.pb.h"
#include "lobbyCommand.pb.h"
#include "lobbyEvent.pb.h"
#include "moveEvents.pb.h"
#include "serverMessage.pb.h"
#include "sessionEvent.pb.h"
#include "sessionCommand.pb.h"

#include <QDebug>

namespace {
template<typename T>
std::shared_ptr<CommandContainer> prepareCommand(const ::google::protobuf::Message &cmd) {
    T typeCmd;
    typeCmd.mutable_command()->PackFrom(cmd);

    auto cont = std::make_shared<CommandContainer>();
    cont->mutable_command()->PackFrom(typeCmd);
    return cont;
}
}

Client::Client(ClientConnection *connection)
    : connection(connection) {
    connection->setParent(this);
    connect(this, &Client::queueCommand, this, &Client::sendCommandContainer);
    connect(this, &Client::sigConnectToHost, this, &Client::doConnectToHost);

    connect(connection, &ClientConnection::messageReady, this, &Client::processServerMessage);
    connect(connection, &ClientConnection::connectionClosed, this, &Client::connectionClosed);
}

void Client::sendSessionCommand(const google::protobuf::Message &cmd) {
    sendCommand(prepareCommand<SessionCommand>(cmd));
}

void Client::sendLobbyCommand(const ::google::protobuf::Message &cmd) {
    sendCommand(prepareCommand<LobbyCommand>(cmd));
}

void Client::sendGameCommand(const ::google::protobuf::Message &cmd) {
    sendCommand(prepareCommand<GameCommand>(cmd));
}

void Client::connectToHost(const QString &hostname, uint16_t port) {
    emit sigConnectToHost(hostname, port);
}

void Client::sendCommandContainer(std::shared_ptr<CommandContainer> command) {
    connection->sendMessage(command);
}

void Client::doConnectToHost(const QString &hostname, uint16_t port) {
    connection->connectToHost(hostname, port);
}

void Client::processServerMessage(std::shared_ptr<ServerMessage> message) {
    try {
    if (message->message().Is<LobbyEvent>()) {
        auto event = std::make_shared<LobbyEvent>();
        message->message().UnpackTo(event.get());
        emit lobbyEventReceived(event);
    } else if (message->message().Is<GameEvent>()) {
        auto event = std::make_shared<GameEvent>();
        message->message().UnpackTo(event.get());
        emit gameEventReceived(event);
    } else if (message->message().Is<SessionEvent>()) {
        auto event = std::make_shared<SessionEvent>();
        message->message().UnpackTo(event.get());
        emit sessionEventReceived(event);
    }
    } catch(const std::exception &) {}
}
