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
    std::unordered_map<int, std::unique_ptr<ServerGame>> mGames;

    int mNextGameId;

public:
    Server();

    QReadWriteLock mGamesLock;
    std::unordered_map<int, std::unique_ptr<ServerGame>>& games() {
        return mGames;
    }
    ServerGame* game(int id);

    void createGame(const CommandCreateGame &cmd, ServerProtocolHandler *client);
    void processGameJoinRequest(const CommandJoinGame &cmd, ServerProtocolHandler *client);

protected:
    int nextGameId();
};
