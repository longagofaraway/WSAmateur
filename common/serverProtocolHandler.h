#pragma once

#include <QObject>

#include <google/protobuf/message.h>

#include "connection.h"

class GameCommand;
class LobbyCommand;
class Server;

class ServerProtocolHandler : public QObject {
    Q_OBJECT
private:
    Server *mServer;
    std::unique_ptr<Connection> mConnection;
    size_t mGameId;
    size_t mPlayerId;

public:
    ServerProtocolHandler(Server *server, std::unique_ptr<Connection> &&connection);

    void sendLobbyEvent(const ::google::protobuf::Message &event);
    void sendProtocolItem(const ::google::protobuf::Message &event);

    void addGameAndPlayer(size_t gameId, size_t playerId);

public slots:
    void processCommand(std::shared_ptr<CommandContainer>);

private:
    void processLobbyCommand(LobbyCommand &cmd);
    void processGameCommand(GameCommand &cmd);
};
