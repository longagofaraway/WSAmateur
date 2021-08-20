#pragma once

#include <memory>
#include <stdexcept>

#include <QObject>

class CommandContainer;
class ServerMessage;

class ClientConnection : public QObject
{
    Q_OBJECT
public:
    virtual void sendMessage(std::shared_ptr<CommandContainer> cont) = 0;

    virtual void connectToHost(const QString &hostname, uint16_t port) {}

signals:
    void messageReady(std::shared_ptr<ServerMessage> message);
};
