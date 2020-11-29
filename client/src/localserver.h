#pragma once

#include <vector>

#include <QObject>

#include "localServerConnection.h"
#include "server.h"

class LocalServer : public Server
{
    Q_OBJECT
public:
    std::shared_ptr<LocalServerConnection> newConnection();
signals:

};
