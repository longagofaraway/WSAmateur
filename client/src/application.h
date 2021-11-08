#pragma once

#include <memory>

#include <QQuickItem>
#include <QThread>

#include "client.h"

class Game;
class EventServerHandshake;
class EventDatabase;

class WSApplication : public QQuickItem
{
    Q_OBJECT
private:
    QThread clientThread;
    std::unique_ptr<Client> client;
    int playerId;
    bool connectionFailed = false;

public:
    WSApplication();
    ~WSApplication();

    Q_INVOKABLE void initGame(Game *game);

signals:
    void startGame();
    void needUpdate();
    void error();
    void loadLobby();

private slots:
    void processSessionEvent(const std::shared_ptr<SessionEvent> event);
    void processLobbyEvent(const std::shared_ptr<LobbyEvent> event);
    void onConnectionClosed();

protected:
    void componentComplete() override;

private:
    void processHandshake(const EventServerHandshake &event);
    void updateDatabase(const EventDatabase &event);
    void sendDatabaseRequest();
    void enterLobby();
    void gameJoined(const EventGameJoined &event);
    void lobbyInfoReceived(const EventLobbyInfo &event);
};

