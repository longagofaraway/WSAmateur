#pragma once

#include <QObject>

#include "connection.h"

class ServerProtocolHandler : public QObject {
    Q_OBJECT
private:
    std::shared_ptr<Connection> mConnection;
public:
    ServerProtocolHandler(std::shared_ptr<Connection> connection)
        : mConnection(connection) {}
};
