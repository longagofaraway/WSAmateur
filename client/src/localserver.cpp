#include "localServer.h"

#include "serverProtocolHandler.h"

std::shared_ptr<LocalServerConnection> LocalServer::newConnection() {
    auto conn = std::make_shared<LocalServerConnection>();
    conn->moveToThread(thread());
    auto client = std::make_shared<ServerProtocolHandler>(this, conn);
    client->moveToThread(thread());
    mClients.emplace_back(client);
    return conn;
}
