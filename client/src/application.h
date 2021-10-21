#pragma once

#include <memory>

#include <QQuickItem>
#include <QThread>

#include "client.h"

class Game;

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

private slots:
    void gameListReceived(const std::shared_ptr<EventGameList> event);
    void gameJoined(const std::shared_ptr<EventGameJoined> event);
    void onConnectionClosed();

protected:
    void componentComplete() override;
};

