#pragma once

#include <vector>

#include <QObject>

#include "server.h"

class LocalServerConnection;

class LocalServer : public Server
{
    Q_OBJECT
public:
    LocalServerConnection* newConnection();
};
