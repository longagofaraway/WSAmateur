#include "networkConnectionManager.h"

#include <iostream>

#include "server.h"
#include "serverProtocolHandler.h"
#include "tcpConnection.h"

NetworkConnectionManager::NetworkConnectionManager(int threads, int tcpPort)
    : threadNumber(threads), tcpPort(tcpPort) {
    connect(&tcpServer, &TcpServer::newConnection, this, &NetworkConnectionManager::handleNewConnection);
    connect(&tcpServer, &QTcpServer::acceptError, this, &NetworkConnectionManager::catchServerError);
}

void NetworkConnectionManager::initialize(Server *server) {
    mServer = server;

    for (int i = 0; i < threadNumber; ++i)
        threadPool.emplace_back(std::make_unique<ConnectionPool>())->startThread();

    if (!tcpServer.listen(QHostAddress::AnyIPv4, tcpPort)) {
        std::cerr << tcpServer.errorString().toStdString() << '\n';
        throw std::runtime_error("");
    }
}

void NetworkConnectionManager::handleNewConnection(qintptr socketDescriptor) {
    auto connPool = findLeastUsedConnectionPool();
    auto conn = std::make_unique<TcpConnection>(socketDescriptor);
    auto handler = mServer->addClient(std::make_unique<ServerProtocolHandler>(mServer, std::move(conn)));
    connPool->addClient(handler);

    // we must create the socket in its thread
    QMetaObject::invokeMethod(handler, "initConnection", Qt::QueuedConnection);
}

ConnectionPool* NetworkConnectionManager::findLeastUsedConnectionPool() {
    int minClientCount = -1;
    size_t poolIndex = -1;
    for (size_t i = 0; i < threadPool.size(); ++i) {
        auto clientCount = threadPool[i]->clientCount();
        if ((poolIndex == -1) || (clientCount < minClientCount)) {
            minClientCount = clientCount;
            poolIndex = i;
        }
    }
    return threadPool[poolIndex].get();
}

void NetworkConnectionManager::catchServerError(QAbstractSocket::SocketError socketError) {
    qDebug() << tcpServer.errorString();
}
