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

private slots:
    void processSessionEvent(const std::shared_ptr<SessionEvent> event);
    void gameListReceived(const std::shared_ptr<EventGameList> event);
    void gameJoined(const std::shared_ptr<EventGameJoined> event);
    void onConnectionClosed();

protected:
    void componentComplete() override;

private:
    void processHandshake(const EventServerHandshake &event);
    void updateDatabase(const EventDatabase &event);
    void sendDatabaseRequest();

};

