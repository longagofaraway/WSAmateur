#pragma once

#include <QObject>

#include "connection.h"

class LocalConnection : public Connection
{
    Q_OBJECT
public:
    explicit LocalConnection() {}
    LocalConnection(LocalConnection *connection);

    //void connect() override {}
    //void close() override {}
    void sendMessage() override;
signals:
    void messageSent();
public slots:
    void messageReceived();
};
