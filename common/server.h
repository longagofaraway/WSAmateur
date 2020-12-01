#pragma once

#include <unordered_map>

#include <QObject>
#include <QMutex>
#include <QReadWriteLock>

#include "serverGame.h"
#include "serverProtocolHandler.h"

class CommandCreateGame;
class CommandJoinGame;

class Server : public QObject
{
    Q_OBJECT
protected:
    std::vector<std::unique_ptr<ServerProtocolHandler>> mClients;
    std::unordered_map<size_t, std::unique_ptr<ServerGame>> mGames;

    size_t mNextGameId;

public:
    Server();

    QReadWriteLock mGamesLock;
    std::unordered_map<size_t, std::unique_ptr<ServerGame>>& games() {
        return mGames;
    }
    ServerGame* game(size_t id);

    void createGame(const CommandCreateGame &cmd, ServerProtocolHandler *client);
    void processGameJoinRequest(const CommandJoinGame &cmd, ServerProtocolHandler *client);

protected:
    size_t nextGameId();
};
