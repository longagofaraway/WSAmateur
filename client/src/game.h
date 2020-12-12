#pragma once

#include <deque>
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
class Client;

class Game : public QQuickItem
{
    Q_OBJECT

private:
    std::unique_ptr<Player> mPlayer;
    std::unique_ptr<Player> mOpponent;
    std::unique_ptr<LocalServer> mLocalServer;
    QThread mClientThread;
    std::vector<std::unique_ptr<Client>> mClients;
    bool mActionInProgress = false;
    std::deque<std::shared_ptr<GameEvent>> mEventQueue;

public:
    Game();
    ~Game();

    Q_INVOKABLE void actionComplete();
    Q_INVOKABLE void sendMulliganFinished();
    Q_INVOKABLE QQuickItem* getZone(QString name, bool opponent) {
        auto zoneName = name.toStdString();
        if (!opponent) {
            return mPlayer->zone(zoneName)->visualItem();
        }

        return mOpponent->zone(zoneName)-> visualItem();
    }

    QQmlEngine* engine() const;
    QQmlContext* context() const;

    void sendGameCommand(const google::protobuf::Message &command, size_t playerId);
    void startUiAction() { mActionInProgress = true; }

public slots:
    void localGameCreated(const std::shared_ptr<EventGameJoined> event);
    void opponentJoined(const std::shared_ptr<EventGameJoined> event);
    void processGameEvent(const std::shared_ptr<GameEvent> event);
    void processGameEventFromQueue();

    void cardSelectedForMulligan(bool selected);
    void cardMoveFinished();

private:
    void startLocalGame();
    void addClient();

    Client* getClientForPlayer(size_t playerId);

protected:
    void componentComplete() override;
};
