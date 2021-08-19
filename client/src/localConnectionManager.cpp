#include "localConnectionManager.h"

#include "localServerConnection.h"

LocalServerConnection* LocalConnectionManager::newConnection() {
    auto conn = std::make_unique<LocalServerConnection>();
    auto connPtr = conn.get();
    auto client = std::make_unique<ServerProtocolHandler>(mServer, std::move(conn));
    client->moveToThread(thread());
    mClients.emplace_back(std::move(client));
    return connPtr;
}
