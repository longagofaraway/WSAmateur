#include "localServer.h"

LocalServer::LocalServer(QObject *parent) : QObject(parent)
{

}

std::shared_ptr<LocalConnection> LocalServer::newConnection() {
    auto conn = std::make_shared<LocalConnection>();
    conn->moveToThread(thread());
    auto client = std::make_shared<ServerProtocolHandler>(conn);
    client->moveToThread(thread());
    mClients.emplace_back(client);
    return conn;
}
