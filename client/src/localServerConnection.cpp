#include "localServerConnection.h"

void LocalServerConnection::messageReceived(std::shared_ptr<CommandContainer> cont) {
    emit messageReady(cont);
}

void LocalServerConnection::sendMessage(std::shared_ptr<ServerMessage> message) {
    emit messageSent(message);
}
