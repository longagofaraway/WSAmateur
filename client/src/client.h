#pragma once

#include <QObject>

#include <google/protobuf/message.h>

#include "commandContainer.pb.h"

#include "clientConnection.h"

class LobbyEvent;
class EventGameJoined;

std::shared_ptr<CommandContainer> prepareLobbyCommand(const ::google::protobuf::Message &cmd);

class Client : public QObject {
    Q_OBJECT
private:
    std::shared_ptr<ClientConnection> mConnection;
public:
    Client(std::shared_ptr<ClientConnection> connection);

    void sendCommand(std::shared_ptr<CommandContainer> command) {
        emit queueCommand(command);
    }

    void sendLobbyCommand(const ::google::protobuf::Message &cmd) {
        sendCommand(prepareLobbyCommand(cmd));
    }

signals:
    void queueCommand(std::shared_ptr<CommandContainer> command);
    void gameJoinedEventReceived(const std::shared_ptr<EventGameJoined> event);

private slots:
    void sendCommandContainer(std::shared_ptr<CommandContainer> command) {
        mConnection->sendMessage(command);
    }

    void processServerMessage(std::shared_ptr<ServerMessage> message);

private:
    void processLobbyEvent(LobbyEvent &event);

protected:
};
