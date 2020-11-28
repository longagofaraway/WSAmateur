#pragma once

#include <vector>

#include <QObject>

#include "localConnection.h"
#include "serverProtocolHandler.h"

class LocalServer : public QObject
{
    Q_OBJECT
private:
    std::vector<std::shared_ptr<ServerProtocolHandler>> mClients;
public:
    explicit LocalServer(QObject *parent = nullptr);

    std::shared_ptr<LocalConnection> newConnection();
signals:

};
