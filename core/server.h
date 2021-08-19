#pragma once

#include <unordered_map>

#include <QObject>
#include <QMutex>
#include <QReadWriteLock>

#include "connectionManager.h"
#include "serverGame.h"
#include "serverProtocolHandler.h"

class CommandCreateGame;
class CommandJoinGame;

class Server : public QObject
{
    Q_OBJECT
protected:
    std::unique_ptr<ConnectionManager> mConnectionManager;
    std::unordered_map<int, std::unique_ptr<ServerGame>> mGames;
    std::vector<std::unique_ptr<ServerProtocolHandler>> mClients;

    int mNextGameId;
    QReadWriteLock mClientsLock;

public:
    Server(std::unique_ptr<ConnectionManager> cm);

    QReadWriteLock mGamesLock;
    ServerGame* game(int id);

    ServerProtocolHandler* addClient(std::unique_ptr<ServerProtocolHandler>);
    void removeClient(ServerProtocolHandler *client);

    void createGame(const CommandCreateGame &cmd, ServerProtocolHandler *client);
    void processGameJoinRequest(const CommandJoinGame &cmd, ServerProtocolHandler *client);

protected:
    int nextGameId();
};
