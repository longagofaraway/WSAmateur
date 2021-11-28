#pragma once

#include <QObject>

#include <google/protobuf/message.h>

#include "commandContainer.pb.h"

#include "clientConnection.h"

class GameEvent;
class LobbyEvent;
class SessionEvent;
class EventGameJoined;
class EventLobbyInfo;

class Client : public QObject
{
    Q_OBJECT
private:
    ClientConnection *mConnection;
    int id_;

public:
    Client(std::unique_ptr<ClientConnection> &&connection);

    void sendCommand(std::shared_ptr<CommandContainer> command) {
        // this should go to a different thread
        emit queueCommand(command);
    }

    int id() const { return id_; }
    void setId(int _id) { id_ = _id; }

    void sendSessionCommand(const ::google::protobuf::Message &cmd);
    void sendLobbyCommand(const ::google::protobuf::Message &cmd);
    void sendGameCommand(const ::google::protobuf::Message &cmd);

    void connectToHost(const std::string &hostname, uint16_t port);

signals:
    void queueCommand(std::shared_ptr<CommandContainer> command);
    void gameEventReceived(const std::shared_ptr<GameEvent> event);
    void sessionEventReceived(const std::shared_ptr<SessionEvent> event);
    void lobbyEventReceived(const std::shared_ptr<LobbyEvent> event);

    void sigConnectToHost(const QString &hostname, uint16_t port);
    void connectionClosed();

private slots:
    void sendCommandContainer(std::shared_ptr<CommandContainer> command);
    void doConnectToHost(const QString &hostname, uint16_t port);

    void processServerMessage(std::shared_ptr<ServerMessage> message);
};
