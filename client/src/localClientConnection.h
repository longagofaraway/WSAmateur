#pragma once

#include <QObject>

#include "localServerConnection.h"
#include "clientConnection.h"

class CommandContainer;

class LocalClientConnection : public ClientConnection
{
    Q_OBJECT
public:
    explicit LocalClientConnection() {}
    LocalClientConnection(LocalServerConnection *target);

    void sendMessage(std::shared_ptr<CommandContainer> cont) override;
signals:
    void messageSent(std::shared_ptr<CommandContainer> cont);

public slots:
    void messageReceived(std::shared_ptr<ServerMessage> message);
};
