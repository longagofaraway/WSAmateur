#include "tcpServer.h"

#include "networkConnectionManager.h"

void TcpServer::incomingConnection(qintptr socketDescriptor) {
    emit newConnection(socketDescriptor);
}
