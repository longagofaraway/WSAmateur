#pragma once

#include <deque>
#include <memory>

#include <QObject>
#include <QQuickItem>
#include <QThread>

#include "client.h"
#include "player.h"
#include "localConnectionManager.h"

class QQmlContext;
class QQmlEngine;
class QQmlContext;
class LocalServer;
class Client;
class Server;
class PlayerInfo;
class GameInfo;
class EventDeckSet;

class Game : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(Player *player READ player CONSTANT FINAL)
    Q_PROPERTY(Player *opponent READ opponent CONSTANT FINAL)

private:
    bool mIsLocal;
    std::unique_ptr<Player> mPlayer;
    std::unique_ptr<Player> mOpponent;
    Client *mClient;
    bool mActionInProgress = false;
    bool mUiActionInProgress = false;
    std::deque<std::shared_ptr<GameEvent>> mEventQueue;

    std::unique_ptr<Server> mLocalServer;
    QThread mLocalServerThread;
    std::vector<std::unique_ptr<Client>> mLocalClients;

    asn::Phase mCurrentPhase = asn::Phase::NotSpecified;

public:
    Game();
    ~Game();

    void startNetworkGame(Client *client, int playerId);
    void startLocalGame();
    void preGameLoaded();

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
    Q_INVOKABLE void sendTakeDamageCommand();
    Q_INVOKABLE void sendEncoreCommand();
    Q_INVOKABLE void sendEndTurn();
    Q_INVOKABLE bool isCounterStep() { return mCurrentPhase == asn::Phase::CounterStep; }
    Q_INVOKABLE void quitGame();
    Q_INVOKABLE QQuickItem* getZone(QString name, bool opponent) {
        auto zoneName = name.toStdString();
        if (zoneName == "marker")
            zoneName = "stage";
        if (!opponent) {
            return mPlayer->zone(zoneName)->visualItem();
        }

        return mOpponent->zone(zoneName)->visualItem();
    }

    Q_INVOKABLE void testAction();

    QQmlEngine* engine() const;
    QQmlContext* context() const;
    Client* client() { return mClient; }

    void pause(int ms);
    void showText(QString mainText, QString subText = "");
    void hideText();

    void sendGameCommand(const google::protobuf::Message &command, int playerId);

    asn::Phase phase() const { return mCurrentPhase; }
    void setPhase(asn::Phase phase) { mCurrentPhase = phase; }
    void startTurn(bool opponent);
    void clockPhase();
    void mainPhase();
    void pauseMainPhase();
    void attackDeclarationStep();
    void attackDeclarationStepFinished();
    void counterStep();
    void endCounterStep();
    void encoreStep();
    void pauseEncoreStep();
    void discardTo7();
    void clearHelpText();
    void endGame(bool victory);
    void playerLeft();

    void setPlayerDeck(const DeckList& deck);

    enum ViewMode {
        RevealMode,
        LookMode,
        MarkerMode
    };
    Q_ENUM(ViewMode)

signals:
    void gameCreated();
    void startGamePreparation();
    void opponentDeckSet(const std::string &xmlDeck);
    void gameEnded();
    void endGamePrematurely(QString reason, QString destination);

public slots:
    void localGameCreated(const EventGameJoined &event);
    void localOpponentJoined(const EventGameJoined &event);
    void processGameEvent(const std::shared_ptr<GameEvent> event);
    void processGameEventFromQueue();
    void processGameEventByOpponent(const std::shared_ptr<GameEvent> event);
    void addOpponent(const PlayerInfo &info);
    void processGameInfo(const GameInfo &game_info);
    void setOpponentDeck(const EventDeckSet &event);
    void onConnectionClosed();

    void cardMoveFinished();

private slots:
    void processLobbyEventLocal(const std::shared_ptr<LobbyEvent> event);
    void processLobbyEventLocal2ndPlayer(const std::shared_ptr<LobbyEvent> event);

private:
    void addLocalClient(LocalConnectionManager *connManager);

    Client* getClientForPlayer(int playerId);

protected:
    void componentComplete() override;
};
