#pragma once

#include <QObject>

#include "connection.h"

class LocalServerConnection : public Connection
{
    Q_OBJECT
public:
    void sendMessage(std::shared_ptr<ServerMessage> message) override;

signals:
    void messageSent(std::shared_ptr<ServerMessage> message);

public slots:
    void messageReceived(std::shared_ptr<CommandContainer> cont);
};
