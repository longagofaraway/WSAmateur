#include "serverProtocolHandler.h"

#include "commandContainer.pb.h"
#include "lobbyCommand.pb.h"
#include "lobbyEvent.pb.h"
#include "serverMessage.pb.h"

#include "server.h"

ServerProtocolHandler::ServerProtocolHandler(Server *server, std::shared_ptr<Connection> connection)
    : mServer(server), mConnection(connection) {
    connect(mConnection.get(), SIGNAL(messageReady(std::shared_ptr<CommandContainer>)),
            this, SLOT(processCommand(std::shared_ptr<CommandContainer>)));
}

void ServerProtocolHandler::processCommand(std::shared_ptr<CommandContainer> cont) {
    if (cont->command().Is<LobbyCommand>()) {
        LobbyCommand lobbyCmd;
        cont->command().UnpackTo(&lobbyCmd);
        processLobbyCommand(lobbyCmd);
    }
}

void ServerProtocolHandler::processLobbyCommand(LobbyCommand &cmd) {
    if (cmd.command().Is<CommandCreateGame>()) {
        CommandCreateGame createCmd;
        cmd.command().UnpackTo(&createCmd);
        mServer->createGame(createCmd, this);
    }
}

void ServerProtocolHandler::sendProtocolItem(const ::google::protobuf::Message &event) {
    auto message = std::make_shared<ServerMessage>();
    message->mutable_message()->PackFrom(event);
    mConnection->sendMessage(message);
}

void ServerProtocolHandler::sendLobbyEvent(const ::google::protobuf::Message &event) {
    LobbyEvent lobbyEvent;
    lobbyEvent.mutable_event()->PackFrom(event);
    sendProtocolItem(lobbyEvent);
}
