#include "localConnection.h"

void LocalConnection::sendMessage() {
    emit messageSent();
}

LocalConnection::LocalConnection(LocalConnection *connection) {
    connect(this, SIGNAL(messageSent()), connection, SLOT(messageReceived()));
    connect(connection, SIGNAL(messageSent()), this, SLOT(messageReceived()));
}

void LocalConnection::messageReceived() {
    emit messageReady();
}
