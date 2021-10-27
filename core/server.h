#pragma once

#include <unordered_map>
#include <unordered_set>

#include <QObject>
#include <QMutex>
#include <QReadWriteLock>

#include "lobbyEvent.pb.h"

#include "connectionManager.h"
#include "serverGame.h"
#include "serverProtocolHandler.h"

class QTimer;
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

    QTimer *mPingClock = nullptr;
    const int mClientKeepalive = 3;

    std::unordered_set<ServerProtocolHandler*> mGameListSubscribers;
    QReadWriteLock mSubscribersLock;
    QTimer *mNotifyClock = nullptr;
    int mSubscribersNotifyInterval = 2;

public:
    Server(std::unique_ptr<ConnectionManager> cm);

    QReadWriteLock mGamesLock;
    EventGameList gameList();
    ServerGame* game(int id);

    ServerProtocolHandler* addClient(std::unique_ptr<ServerProtocolHandler>);
    void removeClient(ServerProtocolHandler *client);

    void createGame(const CommandCreateGame &cmd, ServerProtocolHandler *client);
    void removeGame(int id);
    void processGameJoinRequest(const CommandJoinGame &cmd, ServerProtocolHandler *client);

    int maxClientInactivityTime() const;
    void sendServerIdentification(ServerProtocolHandler *client);
    void sendDatabase(ServerProtocolHandler *client);
    void addGameListSubscriber(ServerProtocolHandler *client);
    void removeGameListSubscriber(ServerProtocolHandler *client);

signals:
    void pingClockTimeout();

protected:
    int nextGameId();

private slots:
    void sendGameList();
};
