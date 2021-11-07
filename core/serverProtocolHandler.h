#pragma once

#include <queue>
#include <unordered_set>

#include <QMutex>
#include <QObject>

#include <google/protobuf/message.h>

#include "connection.h"

class GameCommand;
class LobbyCommand;
class SessionCommand;
class Server;
class QTimer;

class CommandCancelInvite;
class CommandAcceptInvite;
class CommandDeclineInvite;

class ServerProtocolHandler : public QObject
{
    Q_OBJECT
private:
    int mId = 0;
    std::string mName;
    bool mInQueue = false;
    bool mHasOutcomingInvite = false;
    int mInviteeId;
    std::unordered_set<int> mReceivedInvites;
    mutable QMutex mInvitesMutex;

    Server *mServer;
    Connection *mConnection;
    int mGameId = 0;
    int mPlayerId = 0;

    QMutex mOutputQueueMutex;
    std::queue<std::shared_ptr<ServerMessage>> mOutputQueue;

    int timeRunning = 0;
    int lastDataReceived = 0;
    QTimer *pingClock;

public:
    ServerProtocolHandler(Server *server, std::unique_ptr<Connection> &&connection);
    ~ServerProtocolHandler();

    int id() const { return mId; }
    void setId(int id) { mId = id; }
    const std::string& name() const { return mName; }
    bool inQueue() const { return mInQueue; }
    void setInQueue(bool value) { mInQueue = value; }
    bool hasOutcomingInvite() const { return mHasOutcomingInvite; }
    void setOutcomingInvite(bool value, int inviteeId = 0);
    int inviteeId() const { return mInviteeId; }
    bool invited() const;

    void sendInvite(ServerProtocolHandler *sender);
    void refuseAllInvites();
    void inviteDeclined(const CommandDeclineInvite &cmd);
    void inviteWithdrawn(ServerProtocolHandler *sender);
    void onAcceptInvite(const CommandAcceptInvite &cmd);
    void inviteAccepted();
    bool removeInvite(int inviterId);

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
