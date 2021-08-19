#pragma once

#include <memory>

#include <QObject>

class CommandContainer;
class ServerMessage;

class Connection : public QObject
{
    Q_OBJECT
public:
    explicit Connection(QObject *parent = nullptr)
        : QObject(parent) {}

    virtual void init() = 0;
    virtual void sendMessage(std::shared_ptr<ServerMessage> message) = 0;
    virtual void flush() = 0;

signals:
    void connectionClosed();
    void messageReady(std::shared_ptr<CommandContainer> cont);
};
