#pragma once

#include <QObject>

class Server;

class ConnectionManager : public QObject
{
    Q_OBJECT
protected:
    Server *mServer;

public:
    virtual ~ConnectionManager() {};

    virtual void initialize(Server *server) = 0;
};
