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

extern std::string gDeck;

class Game : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(Player *player READ player CONSTANT FINAL)
    Q_PROPERTY(Player *opponent READ opponent CONSTANT FINAL)

private:
    std::unique_ptr<Player> mPlayer;
    std::unique_ptr<Player> mOpponent;
    std::unique_ptr<LocalServer> mLocalServer;
    QThread mClientThread;
    std::vector<std::unique_ptr<Client>> mClients;
    bool mActionInProgress = false;
    bool mUiActionInProgress = false;
    std::deque<std::shared_ptr<GameEvent>> mEventQueue;

    asn::Phase mCurrentPhase = asn::Phase::NotSpecified;

public:
    Game();
    ~Game();

    //accessing players from qml
    Player* player() { return mPlayer.get(); }
    Player* opponent() { return mOpponent.get(); }

    // actions - network initiated ui actions
    // ui actions - user initiated ui actions, they cannot be delayed
    // and can start during other actions
    Q_INVOKABLE void actionComplete();
    Q_INVOKABLE void uiActionComplete();
    Q_INVOKABLE void startAction() { mActionInProgress = true; }
    Q_INVOKABLE void startUiAction() { mUiActionInProgress = true; }
    Q_INVOKABLE void sendMulliganFinished();
    Q_INVOKABLE void sendClockPhaseFinished();
    Q_INVOKABLE void sendMainPhaseFinished();
    Q_INVOKABLE void sendClimaxPhaseCommand();
    Q_INVOKABLE void sendTakeDamageCommand();
    Q_INVOKABLE void sendEncoreCommand();
    Q_INVOKABLE void sendEndTurn();
    Q_INVOKABLE QQuickItem* getZone(QString name, bool opponent) {
        auto zoneName = name.toStdString();
        if (!opponent) {
            return mPlayer->zone(zoneName)->visualItem();
        }

        return mOpponent->zone(zoneName)->visualItem();
    }

    Q_INVOKABLE void testAction();

    QQmlEngine* engine() const;
    QQmlContext* context() const;

    void pause(int ms);
    void showText(QString mainText, QString subText);
    void hideText();

    void sendGameCommand(const google::protobuf::Message &command, int playerId);

    asn::Phase phase() const { return mCurrentPhase; }
    void setPhase(asn::Phase phase) { mCurrentPhase = phase; }
    void startTurn(bool opponent);
    void clockPhase();
    void mainPhase();
    void attackDeclarationStep();
    void attackDeclarationStepFinished();
    void counterStep();
    void encoreStep();
    void pauseEncoreStep();
    void discardTo7();
    void clearHelpText();
    void endGame(bool victory);

public slots:
    void localGameCreated(const std::shared_ptr<EventGameJoined> event);
    void opponentJoined(const std::shared_ptr<EventGameJoined> event);
    void processGameEvent(const std::shared_ptr<GameEvent> event);
    void processGameEventFromQueue();
    void processGameEventByOpponent(const std::shared_ptr<GameEvent> event);

    void cardMoveFinished();

private:
    void startLocalGame();
    void addClient();

    Client* getClientForPlayer(int playerId);

protected:
    void componentComplete() override;
};
