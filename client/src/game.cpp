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

std::string gDeck = R"delim(<?xml version="1.0" encoding="UTF-8"?>
    <deck version="1">
    <deckname>Vivid Green 2</deckname>
    <comments></comments>
    <main>
        <card number="1" code="IMC/W43-127"/>
        <card number="20" code="KGL/S79-001"/>
        <card number="20" code="KGL/S79-003"/>
        <card number="1" code="IMC/W43-046"/>
        <card number="1" code="IMC/W43-009"/>
        <card number="1" code="IMC/W43-111"/>
        <card number="1" code="IMC/W43-091"/>
    </main>
</deck>)delim";

std::string gOppDeck = R"delim(<?xml version="1.0" encoding="UTF-8"?>
    <deck version="1">
    <deckname>Vivid Green 2</deckname>
    <comments></comments>
    <main>
        <card number="1" code="IMC/W43-127"/>
        <card number="1" code="IMC/W43-046"/>
        <card number="42" code="IMC/W43-009"/>
        <card number="1" code="IMC/W43-111"/>
        <card number="1" code="IMC/W43-091"/>
    </main>
</deck>)delim";

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

    mPlayer->setDeck(gDeck);
    mOpponent->setDeck(gOppDeck);
    CommandSetDeck cmd;
    cmd.set_deck(gDeck);
    mClients[0]->sendGameCommand(cmd);
    CommandSetDeck cmd2;
    cmd2.set_deck(gOppDeck);
    mClients[1]->sendGameCommand(cmd2);

    CommandReadyToStart readyCmd;
    readyCmd.set_ready(true);
    mClients[0]->sendGameCommand(readyCmd);
    mClients[1]->sendGameCommand(readyCmd);
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

void Game::sendEncoreCommand() {
    mPlayer->sendEncoreCommand();
}

void Game::sendEndTurn() {
    mPlayer->sendEndTurnCommand();
}

QQmlEngine* Game::engine() const { return qmlEngine(parentItem()); }
QQmlContext* Game::context() const { return qmlContext(parentItem()); }

void Game::pause(int ms) {
    QMetaObject::invokeMethod(this, "pause", Q_ARG(QVariant, ms));
}

void Game::showText(QString mainText, QString subText) {
    QMetaObject::invokeMethod(this, "showText", Q_ARG(QVariant, mainText), Q_ARG(QVariant, subText));
}

void Game::hideText() {
    QMetaObject::invokeMethod(this, "hideText");
}

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
    mOpponent->setActivePlayer(opponent);
    mPlayer->setActivePlayer(!opponent);
    mActionInProgress = true;
    QMetaObject::invokeMethod(this, "startTurn", Q_ARG(QVariant, opponent));
}

void Game::clockPhase() {
    QMetaObject::invokeMethod(this, "clockPhase");
}

void Game::mainPhase() {
    QMetaObject::invokeMethod(this, "mainPhase");
}

void Game::pauseMainPhase() {
    QMetaObject::invokeMethod(this, "pauseMainPhase");
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

void Game::encoreStep() {
    QMetaObject::invokeMethod(this, "encoreStep");
}

void Game::pauseEncoreStep() {
    QMetaObject::invokeMethod(this, "pauseEncoreStep");
}

void Game::discardTo7() {
    QMetaObject::invokeMethod(this, "discardTo7");
}

void Game::clearHelpText() {
    QMetaObject::invokeMethod(this, "clearHelpText");
}

void Game::endGame(bool victory) {
    QMetaObject::invokeMethod(this, "endGame", Q_ARG(QVariant, victory));

    disconnect(mClients.front().get(), SIGNAL(gameEventReceived(const std::shared_ptr<GameEvent>)),
               this, SLOT(processGameEvent(const std::shared_ptr<GameEvent>)));
    disconnect(mClients.back().get(), SIGNAL(gameEventReceived(const std::shared_ptr<GameEvent>)),
               this, SLOT(processGameEventByOpponent(const std::shared_ptr<GameEvent>)));
    mEventQueue.clear();
}



void Game::testAction() {
    mPlayer->testAction();
    mOpponent->testAction();
}


#include "cardModel.h"
void Game::processGameEventByOpponent(const std::shared_ptr<GameEvent> event) {
    if (mOpponent->id() != event->playerid())
        return;

    static CardModel hand;
    if (event->event().Is<EventInitialHand>()) {
        EventInitialHand ev;
        event->event().UnpackTo(&ev);
        for (int i = 0; i < ev.codes_size(); ++i) {
            mOpponent->zone("deck")->model().removeCard(mOpponent->zone("deck")->model().count());
            hand.addCard(ev.codes(i));
        }
        CommandMulligan cmd;
        //cmd.add_ids(0);
        //cmd.add_ids(2);
        QTimer::singleShot(100, this, [this, cmd]() { sendGameCommand(cmd, mOpponent->id()); });
    } else if (event->event().Is<EventClockPhase>()) {
        CommandClockPhase cmd;
        cmd.set_count(0);
        sendGameCommand(cmd, mOpponent->id());
    } else if (event->event().Is<EventMainPhase>()) {
        mOpponent->playCards(hand);
        sendGameCommand(CommandAttackPhase(), mOpponent->id());
    } else if (event->event().Is<EventPlayCard>()) {
        EventPlayCard ev;
        event->event().UnpackTo(&ev);
        hand.removeCard(ev.handid());
        sendGameCommand(CommandAttackPhase(), mOpponent->id());
        //if (mOpponent->playCards(hand))
        //    sendGameCommand(CommandAttackPhase(), mOpponent->id());
    } else if (event->event().Is<EventAttackDeclarationStep>()) {
        //timing problems
        //mOpponent->attackWithAll();

        sendGameCommand(CommandEncoreStep(), mOpponent->id());
    } else if (event->event().Is<EventCounterStep>()) {
        CommandTakeDamage cmd;
        sendGameCommand(cmd, mOpponent->id());
    } else if (event->event().Is<EventLevelUp>()) {
        CommandLevelUp cmd;
        cmd.set_clockid(0);
        sendGameCommand(cmd, mOpponent->id());
    } else if (event->event().Is<EventEncoreStep>()) {
        CommandEndTurn cmd;
        sendGameCommand(cmd, mOpponent->id());
    } else if (event->event().Is<EventMoveCard>()) {
        EventMoveCard ev;
        event->event().UnpackTo(&ev);
        if (ev.startzone() == "deck" && ev.targetzone() == "hand") {
            hand.addCard(ev.code());
        }
    } else if (event->event().Is<EventDiscardDownTo7>()) {
        CommandMoveCard cmd;
        cmd.set_startid(0);
        cmd.set_startzone("hand");
        cmd.set_targetzone("wr");
        sendGameCommand(cmd, mOpponent->id());
    }
}
