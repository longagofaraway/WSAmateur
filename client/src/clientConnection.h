#pragma once

#include <QObject>

class CommandContainer;
class ServerMessage;

class ClientConnection : public QObject
{
    Q_OBJECT
public:
    virtual void sendMessage(std::shared_ptr<CommandContainer> cont) = 0;

signals:
    void messageReady(std::shared_ptr<ServerMessage> message);
};
