#pragma once

#include <memory>
#include <vector>

#include "connectionManager.h"
#include "connectionPool.h"
#include "tcpServer.h"

class NetworkConnectionManager : public ConnectionManager
{
    Q_OBJECT
private:
    std::vector<std::unique_ptr<ConnectionPool>> threadPool;
    int threadNumber;
    int tcpPort;
    TcpServer tcpServer;

public:
    NetworkConnectionManager(int threads, int tcpPort);

    void initialize(Server *server) override;
    ConnectionPool* findLeastUsedConnectionPool();
    bool isLocal() const override { return false; };

private slots:
    void handleNewConnection(qintptr socketDescriptor);
    void catchServerError(QAbstractSocket::SocketError socketError);
};
