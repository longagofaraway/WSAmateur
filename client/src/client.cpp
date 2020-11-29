#include "client.h"

#include "lobbyCommand.pb.h"
#include "lobbyEvent.pb.h"
#include "serverMessage.pb.h"

std::shared_ptr<CommandContainer> prepareLobbyCommand(const ::google::protobuf::Message &cmd) {
    LobbyCommand lobbyCmd;
    lobbyCmd.mutable_command()->PackFrom(cmd);

    auto cont = std::make_shared<CommandContainer>();
    cont->mutable_command()->PackFrom(lobbyCmd);
    return cont;
}

Client::Client(std::shared_ptr<ClientConnection> connection)
    : mConnection(connection) {
    connect(this, SIGNAL(queueCommand(std::shared_ptr<CommandContainer>)),
        this, SLOT(sendCommandContainer(std::shared_ptr<CommandContainer>)));
    connect(mConnection.get(), SIGNAL(messageReady(std::shared_ptr<ServerMessage>)),
            this, SLOT(processServerMessage(std::shared_ptr<ServerMessage>)));
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
        emit gameJoinedEventReceived(ev);
    }
}
