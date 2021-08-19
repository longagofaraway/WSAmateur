#pragma once

#include <atomic>
#include <vector>

#include <QThread>

class ServerProtocolHandler;

class ConnectionPool
{
public:
    QThread thread;
    std::atomic<int> _clientCount = 0;

public:
    ConnectionPool() {}

    void startThread() { thread.start(); }
    void addClient(ServerProtocolHandler *client);
    int clientCount();
};
