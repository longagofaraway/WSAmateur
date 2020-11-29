#pragma once

#include <unordered_map>

#include <QObject>
#include <QMutex>
#include <QReadWriteLock>

class ServerProtocolHandler;
class ServerGame;
class CommandCreateGame;

class Server : public QObject
{
    Q_OBJECT
protected:
    std::vector<std::shared_ptr<ServerProtocolHandler>> mClients;
    std::unordered_map<size_t, std::shared_ptr<ServerGame>> mGames;
    QReadWriteLock mGamesLock;

    QMutex mNextGameIdMutex;
    size_t mNextGameId;

public:
    Server();

    void createGame(const CommandCreateGame &cmd, ServerProtocolHandler *client);

protected:
    size_t nextGameId();
};
