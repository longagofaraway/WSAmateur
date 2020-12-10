#pragma once

#include <QObject>

#include <google/protobuf/message.h>

#include "commandContainer.pb.h"

#include "clientConnection.h"

class GameEvent;
class LobbyEvent;
class EventGameJoined;

class Client : public QObject
{
    Q_OBJECT
private:
    std::unique_ptr<ClientConnection> mConnection;
public:
    Client(std::unique_ptr<ClientConnection> &&connection);

    void sendCommand(std::shared_ptr<CommandContainer> command) {
        // this should going to different thread
        emit queueCommand(command);
    }

    void sendLobbyCommand(const ::google::protobuf::Message &cmd);
    void sendGameCommand(const ::google::protobuf::Message &cmd);

signals:
    void queueCommand(std::shared_ptr<CommandContainer> command);
    void gameJoinedEventReceived(const std::shared_ptr<EventGameJoined> event);
    void gameEventReceived(const std::shared_ptr<GameEvent> event);

private slots:
    void sendCommandContainer(std::shared_ptr<CommandContainer> command) {
        mConnection->sendMessage(command);
    }

    void processServerMessage(std::shared_ptr<ServerMessage> message);

private:
    void processLobbyEvent(LobbyEvent &event);

protected:
};
