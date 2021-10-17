#include "connectionPool.h"

#include "serverProtocolHandler.h"

ConnectionPool::~ConnectionPool() {
    thread.quit();
    thread.wait();
}

void ConnectionPool::addClient(ServerProtocolHandler *client) {
    _clientCount.fetch_add(1, std::memory_order::relaxed);
    client->moveToThread(&thread);
}

int ConnectionPool::clientCount() {
    return _clientCount.load(std::memory_order::relaxed);
}
