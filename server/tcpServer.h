#pragma once

#include <QTcpServer>

class NetworkConnectionManager;

class TcpServer : public QTcpServer
{
    Q_OBJECT
public:
    void incomingConnection(qintptr socketDescriptor) override;
signals:
    void newConnection(qintptr socketDescriptor);
};
