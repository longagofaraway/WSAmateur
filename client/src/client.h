#pragma once

#include <QObject>

#include "connection.h"

class Client : public QObject {
    Q_OBJECT
private:
    std::shared_ptr<Connection> mConnection;
public:
    Client(std::shared_ptr<Connection> connection)
        : mConnection(connection) {}

protected:
};
