#include "localServer.h"

#include "localServerConnection.h"
#include "serverProtocolHandler.h"

LocalServerConnection* LocalServer::newConnection() {
    auto conn = std::make_unique<LocalServerConnection>();
    conn->moveToThread(thread());
    auto connPtr = conn.get();
    auto client = std::make_unique<ServerProtocolHandler>(this, std::move(conn));
    client->moveToThread(thread());
    mClients.emplace_back(std::move(client));
    return connPtr;
}
