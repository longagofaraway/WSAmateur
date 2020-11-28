#pragma once

#include <QObject>

#include "connection.h"

class RemoteConnection : public Connection
{
    Q_OBJECT
public:
    explicit RemoteConnection() {}

    //void connect() override {}
    //void close() override {}
    void sendMessage() override {}
signals:

};
