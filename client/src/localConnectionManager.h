#pragma once

#include <memory>
#include <vector>

#include <QObject>

#include "connectionManager.h"
#include "serverProtocolHandler.h"

class LocalServerConnection;

class LocalConnectionManager : public ConnectionManager
{
    Q_OBJECT
private:
    std::vector<std::unique_ptr<ServerProtocolHandler>> mClients;

public:
    LocalServerConnection* newConnection();
    void initialize(Server *server) override { mServer = server; }
    bool isLocal() const override { return true; };
};
