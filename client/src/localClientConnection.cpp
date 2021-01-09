#include "localClientConnection.h"

void LocalClientConnection::sendMessage(std::shared_ptr<CommandContainer> cont) {
    emit messageSent(cont);
}

LocalClientConnection::LocalClientConnection(LocalServerConnection *target) {
    connect(this, SIGNAL(messageSent(std::shared_ptr<CommandContainer>)),
            target, SLOT(messageReceived(std::shared_ptr<CommandContainer>)));
    connect(target, SIGNAL(messageSent(std::shared_ptr<ServerMessage>)),
            this, SLOT(messageReceived(std::shared_ptr<ServerMessage>)));
}

void LocalClientConnection::messageReceived(std::shared_ptr<ServerMessage> cont) {
    emit messageReady(cont);
}
