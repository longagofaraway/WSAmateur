#pragma once

#include <memory>

#include <QObject>
#include <QQuickItem>
#include <QThread>

#include "client.h"
#include "player.h"
#include "localServer.h"

class QQmlContext;
class QQmlEngine;
class QQmlContext;
class LocalServer;
class QThread;
class Client;


class Game : public QQuickItem {
    Q_OBJECT
private:
    std::unique_ptr<Player> mPlayer;
    std::unique_ptr<Player> mOpponent;
    std::unique_ptr<LocalServer> mLocalServer;
    std::unique_ptr<QThread> mClientThread;
    std::vector<std::unique_ptr<Client>> mClients;

public:
    ~Game();

    QQmlEngine* engine() const;
    QQmlContext* context() const;

private:
    void startLocalGame();
    void addClient();

protected:
    void componentComplete() override;
};
