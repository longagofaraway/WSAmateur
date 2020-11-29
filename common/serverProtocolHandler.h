#pragma once

#include <QObject>

#include <google/protobuf/message.h>

#include "connection.h"

class LobbyCommand;
class Server;

class ServerProtocolHandler : public QObject {
    Q_OBJECT
private:
    Server *mServer;
    std::shared_ptr<Connection> mConnection;
public:
    ServerProtocolHandler(Server *server, std::shared_ptr<Connection> connection);

    void sendLobbyEvent(const ::google::protobuf::Message &event);
    void sendProtocolItem(const ::google::protobuf::Message &event);

public slots:
    void processCommand(std::shared_ptr<CommandContainer>);

private:
    void processLobbyCommand(LobbyCommand &cmd);
};
