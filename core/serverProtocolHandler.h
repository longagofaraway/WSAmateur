#pragma once

#include <queue>

#include <QMutex>
#include <QObject>

#include <google/protobuf/message.h>

#include "connection.h"

class GameCommand;
class LobbyCommand;
class SessionCommand;
class Server;
class QTimer;

class ServerProtocolHandler : public QObject
{
    Q_OBJECT
private:
    Server *mServer;
    Connection *mConnection;
    int mGameId;
    int mPlayerId;

    QMutex mOutputQueueMutex;
    std::queue<std::shared_ptr<ServerMessage>> mOutputQueue;

    int timeRunning = 0;
    int lastDataReceived = 0;
    QTimer *pingClock;

public:
    ServerProtocolHandler(Server *server, std::unique_ptr<Connection> &&connection);
    ~ServerProtocolHandler();

    void sendSessionEvent(const ::google::protobuf::Message &event);
    void sendLobbyEvent(const ::google::protobuf::Message &event);
    void sendGameEvent(const ::google::protobuf::Message &event, int playerId);
    void sendProtocolItem(const ::google::protobuf::Message &event);

    void addGameAndPlayer(int gameId, int playerId);

signals:
    void outputQueueChanged();

public slots:
    void processCommand(std::shared_ptr<CommandContainer>);
    void initConnection();

private slots:
    void flushOutputQueue();
    void onConnectionClosed();
    void pingClockTimeout();

private:
    void processLobbyCommand(LobbyCommand &cmd);
    void processGameCommand(GameCommand &cmd);
    void processSessionCommand(const SessionCommand &cmd);
};
