#include "game.h"

#include "localClientConnection.h"
#include "localServer.h"

#include "gameCommand.pb.h"
#include "gameEvent.pb.h"
#include "lobbyCommand.pb.h"
#include "lobbyEvent.pb.h"
#include "moveCommands.pb.h"
#include "moveEvents.pb.h"
#include "phaseCommand.pb.h"
#include "phaseEvent.pb.h"

#include <QTimer>
#include <QDebug>

Game::Game() {
    qRegisterMetaType<std::shared_ptr<EventGameJoined>>("std::shared_ptr<EventGameJoined>");
    qRegisterMetaType<std::shared_ptr<GameEvent>>("std::shared_ptr<GameEvent>");
}

Game::~Game() {
    mClientThread.quit();
    mClientThread.wait();
}

void Game::actionComplete() {
    mActionInProgress = false;
    QMetaObject::invokeMethod(this, "processGameEventFromQueue", Qt::QueuedConnection);
}

void Game::uiActionComplete() {
    mUiActionInProgress = false;
    QMetaObject::invokeMethod(this, "processGameEventFromQueue", Qt::QueuedConnection);
}

void Game::componentComplete() {
    QQuickItem::componentComplete();

    startLocalGame();
}

void Game::startLocalGame() {
    mLocalServer = std::make_unique<LocalServer>();
    mLocalServer->moveToThread(&mClientThread);

    addClient();
    addClient();

    mClientThread.start();

    connect(mClients.front().get(), SIGNAL(gameEventReceived(const std::shared_ptr<GameEvent>)),
            this, SLOT(processGameEvent(const std::shared_ptr<GameEvent>)));
    connect(mClients.front().get(), SIGNAL(gameJoinedEventReceived(const std::shared_ptr<EventGameJoined>)),
            this, SLOT(localGameCreated(const std::shared_ptr<EventGameJoined>)));

    connect(mClients.back().get(), SIGNAL(gameEventReceived(const std::shared_ptr<GameEvent>)),
            this, SLOT(processGameEventByOpponent(const std::shared_ptr<GameEvent>)));

    CommandCreateGame command;
    command.set_description("hah");
    mClients.front()->sendLobbyCommand(command);
}

void Game::addClient() {
    auto serverConnection = mLocalServer->newConnection();
    auto localConnection = std::make_unique<LocalClientConnection>(serverConnection);
    localConnection->moveToThread(&mClientThread);

    auto client = std::make_unique<Client>(std::move(localConnection));
    client->moveToThread(&mClientThread);
    mClients.emplace_back(std::move(client));
}

void Game::localGameCreated(const std::shared_ptr<EventGameJoined> event) {
    mPlayer = std::make_unique<Player>(event->playerid(), this, false);

    connect(mClients.back().get(), SIGNAL(gameJoinedEventReceived(const std::shared_ptr<EventGameJoined>)),
            this, SLOT(opponentJoined(const std::shared_ptr<EventGameJoined>)));

    CommandJoinGame command;
    command.set_gameid(event->gameid());
    mClients.back()->sendLobbyCommand(command);
}

void Game::opponentJoined(const std::shared_ptr<EventGameJoined> event) {
    mOpponent = std::make_unique<Player>(event->playerid(), this, true);

    std::string deck = R"delim(<?xml version="1.0" encoding="UTF-8"?>
        <deck version="1">
        <deckname>Vivid Green 2</deckname>
        <comments></comments>
        <main>
            <card number="8" code="IMC/W43-127"/>
            <card number="16" code="IMC/W43-046"/>
            <card number="10" code="IMC/W43-009"/>
            <card number="8" code="IMC/W43-111"/>
            <card number="8" code="IMC/W43-091"/>
        </main>
    </deck>)delim";

    CommandSetDeck cmd;
    cmd.set_deck(deck);
    CommandReadyToStart cmd2;
    cmd2.set_ready(true);
    for (auto &client: mClients) {
        client->sendGameCommand(cmd);
        client->sendGameCommand(cmd2);
    }
}

void Game::processGameEvent(const std::shared_ptr<GameEvent> event) {
    mEventQueue.push_back(event);
    processGameEventFromQueue();
}

void Game::processGameEventFromQueue() {
    if (mActionInProgress || mUiActionInProgress || mEventQueue.empty())
        return;

    auto event = mEventQueue.front();
    mEventQueue.pop_front();
    if (mPlayer->id() == event->playerid())
        mPlayer->processGameEvent(event);
    else
        mOpponent->processGameEvent(event);

    if (!mEventQueue.empty())
        QMetaObject::invokeMethod(this, "processGameEventFromQueue", Qt::QueuedConnection);
}

void Game::cardSelectedForMulligan(bool selected) {
    QMetaObject::invokeMethod(this, "changeCardCountForMulligan", Q_ARG(QVariant, selected));
}

void Game::cardSelectedForClock(bool selected) {
    QMetaObject::invokeMethod(this, "changeCardCountForClock", Q_ARG(QVariant, selected));
}

void Game::cardMoveFinished() {
    sender()->deleteLater();
}

void Game::sendMulliganFinished() {
    mPlayer->mulliganFinished();
}

void Game::sendClockPhaseFinished() {
    mPlayer->clockPhaseFinished();
}

void Game::sendMainPhaseFinished() {
    mPlayer->mainPhaseFinished();
}

void Game::sendClimaxPhaseCommand() {
    mPlayer->sendClimaxPhaseCommand();
}

void Game::sendTakeDamageCommand() {
    mPlayer->sendTakeDamageCommand();
}

QQmlEngine* Game::engine() const { return qmlEngine(parentItem()); }
QQmlContext* Game::context() const { return qmlContext(parentItem()); }

Client* Game::getClientForPlayer(int playerId) {
    if (mClients.size() > 1)
        return mClients.at(playerId - 1).get();
    else if (mClients.empty())
        return nullptr;
    else
        return mClients.front().get();
}

void Game::sendGameCommand(const google::protobuf::Message &command, int playerId) {
    Client *client = getClientForPlayer(playerId);
    if (!client)
        return;

    client->sendGameCommand(command);
}

void Game::startTurn(bool opponent) {
    if (opponent) {
        mOpponent->setActivePlayer(true);
        mPlayer->setActivePlayer(false);
    } else {
        mOpponent->setActivePlayer(false);
        mPlayer->setActivePlayer(true);
    }
    mActionInProgress = true;
    QMetaObject::invokeMethod(this, "startTurn", Q_ARG(QVariant, opponent));
}

void Game::clockPhase() {
    QMetaObject::invokeMethod(this, "clockPhase");
}

void Game::mainPhase() {
    QMetaObject::invokeMethod(this, "mainPhase");
}

void Game::attackDeclarationStep() {
    QMetaObject::invokeMethod(this, "attackDeclarationStep");
}

void Game::attackDeclarationStepFinished() {
    QMetaObject::invokeMethod(this, "attackDeclarationStepFinished");
}

void Game::counterStep() {
    QMetaObject::invokeMethod(this, "counterStep");
}

void Game::levelUp() {
    QMetaObject::invokeMethod(this, "levelUp");
}

void Game::endLevelUp() {
    QMetaObject::invokeMethod(this, "endLevelUp");
}



void Game::testAction() {
    mPlayer->testAction();
    mOpponent->testAction();
}

void Game::processGameEventByOpponent(const std::shared_ptr<GameEvent> event) {
    if (event->event().Is<EventInitialHand>()) {
        EventInitialHand ev;
        event->event().UnpackTo(&ev);
        CommandMulligan cmd;
        //cmd.add_ids(0);
        //cmd.add_ids(2);
        QTimer::singleShot(100, this, [this, cmd]() { sendGameCommand(cmd, mOpponent->id()); });
    } else if (event->event().Is<EventClockPhase>()) {
        CommandClockPhase cmd;
        cmd.set_count(0);
        sendGameCommand(cmd, mOpponent->id());
    } else if (event->event().Is<EventMainPhase>()) {
        CommandPlayCard cmd;
        cmd.set_handid(0);
        sendGameCommand(cmd, mOpponent->id());
        CommandAttackPhase cmd2;
        sendGameCommand(cmd2, mOpponent->id());
    } else if (event->event().Is<EventAttackDeclarationStep>()) {
        CommandDeclareAttack cmd;
        cmd.set_stageid(0);
        cmd.set_attacktype(AttackType::FrontAttack);
        sendGameCommand(cmd, mOpponent->id());
    } else if (event->event().Is<EventCounterStep>()) {
        CommandTakeDamage cmd;
        sendGameCommand(cmd, mOpponent->id());
    }
}
