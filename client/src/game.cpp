#include "game.h"

#include "abilityCommands.pb.h"
#include "abilityEvents.pb.h"
#include "gameCommand.pb.h"
#include "gameEvent.pb.h"
#include "lobbyCommand.pb.h"
#include "lobbyEvent.pb.h"
#include "moveCommands.pb.h"
#include "moveEvents.pb.h"
#include "phaseCommand.pb.h"
#include "phaseEvent.pb.h"

#include "cardDatabase.h"
#include "localClientConnection.h"
#include "localConnectionManager.h"
#include "remoteClientConnection.h"
#include "server.h"

#include <QTimer>
#include <QDebug>

std::string gDeck = R"delim(<?xml version="1.0" encoding="UTF-8"?>
    <deck version="1">
    <deckname>Vivid Green 2</deckname>
    <comments></comments>
    <main>
        <card number="21" code="KGL/S79-071"/>
        <card number="20" code="KGL/S79-077"/>
        <card number="1" code="KGL/S79-T04"/>
    </main>
</deck>)delim";

std::string gOppDeck = R"delim(<?xml version="1.0" encoding="UTF-8"?>
    <deck version="1">
    <deckname>Vivid Green 2</deckname>
    <comments></comments>
    <main>
        <card number="40" code="TST/S99-001"/>
    </main>
</deck>)delim";

Game::Game() {
}

Game::~Game() {
    if (!mIsLocal)
        disconnect(mClient, &Client::connectionClosed, this, &Game::onConnectionClosed);
    for (auto &client: mLocalClients)
        client.release()->deleteLater();
    if (mLocalServerThread.isRunning()) {
        mLocalServerThread.quit();
        mLocalServerThread.wait();
    }
}

void Game::startNetworkGame(Client *client, int playerId) {
    mIsLocal = false;
    mClient = client;
    connect(mClient, &Client::connectionClosed, this, &Game::onConnectionClosed);

    mPlayer = std::make_unique<Player>(playerId, this, false);

    emit startGamePreparation();
}

void Game::preGameLoaded() {
    connect(mClient, &Client::gameEventReceived, this, &Game::processGameEvent);
    mClient->sendGameCommand(CommandGetGameInfo());
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

    emit gameCreated();
}

void Game::addOpponent(const PlayerInfo &info) {
    if (mOpponent)
        return;
    mOpponent = std::make_unique<Player>(info.id(), this, true);
    mOpponent->setDeck(info.deck());
    emit opponentDeckSet(info.deck());

    // for testing
    /*mPlayer->setDeck(gDeck);
    CommandSetDeck cmd;
    cmd.set_deck(gDeck);
    mClient->sendGameCommand(cmd);

    CommandReadyToStart readyCmd;
    readyCmd.set_ready(true);
    mClient->sendGameCommand(readyCmd);*/
}

void Game::processGameInfo(const GameInfo &game_info) {
    for (int i = 0; i < game_info.players_size(); ++i) {
        if (mPlayer->id() == game_info.players(i).id())
            continue;

        addOpponent(game_info.players(i));
    }
}

void Game::setOpponentDeck(const EventDeckSet &event) {
    if (!mOpponent || event.player_id() != mOpponent->id() || mOpponent->deckSet())
        return;

    mOpponent->setDeck(event.deck());
    emit opponentDeckSet(event.deck());
}

void Game::onConnectionClosed() {
    emit endGamePrematurely("Connection error", "start");
}

void Game::startLocalGame() {
    mIsLocal = true;
    auto connManagerPtr = std::make_unique<LocalConnectionManager>();
    auto connManager = connManagerPtr.get();
    connManager->moveToThread(&mLocalServerThread);

    mLocalServer = std::make_unique<Server>(std::move(connManagerPtr));
    mLocalServer->moveToThread(&mLocalServerThread);

    addLocalClient(connManager);
    addLocalClient(connManager);

    mLocalServerThread.start();

    connect(mLocalClients.front().get(), SIGNAL(gameEventReceived(const std::shared_ptr<GameEvent>)),
            this, SLOT(processGameEvent(const std::shared_ptr<GameEvent>)));
    connect(mLocalClients.front().get(), SIGNAL(lobbyEventReceived(const std::shared_ptr<LobbyEvent>)),
            this, SLOT(processLobbyEventLocal(const std::shared_ptr<LobbyEvent>)));

    connect(mLocalClients.back().get(), SIGNAL(gameEventReceived(const std::shared_ptr<GameEvent>)),
            this, SLOT(processGameEventByOpponent(const std::shared_ptr<GameEvent>)));

    CommandCreateGame command;
    command.set_description("hah");
    mLocalClients.front()->sendLobbyCommand(command);
}

void Game::addLocalClient(LocalConnectionManager *connManager) {
    auto serverConnection = connManager->newConnection();
    auto localConnection = new LocalClientConnection(serverConnection);

    auto client = std::make_unique<Client>(localConnection);
    client->moveToThread(&mLocalServerThread);
    mLocalClients.emplace_back(std::move(client));
}


void Game::localGameCreated(const EventGameJoined &event) {
    mPlayer = std::make_unique<Player>(event.player_id(), this, false);

    connect(mLocalClients.back().get(), SIGNAL(lobbyEventReceived(const std::shared_ptr<LobbyEvent>)),
            this, SLOT(processLobbyEventLocal2ndPlayer(const std::shared_ptr<LobbyEvent>)));

    CommandJoinGame command;
    command.set_game_id(event.game_id());
    mLocalClients.back()->sendLobbyCommand(command);
}

void Game::localOpponentJoined(const EventGameJoined &event) {
    mOpponent = std::make_unique<Player>(event.player_id(), this, true);

    mPlayer->setDeck(gDeck);
    mOpponent->setDeck(gOppDeck);
    CommandSetDeck cmd;
    cmd.set_deck(gDeck);
    mLocalClients[0]->sendGameCommand(cmd);
    CommandSetDeck cmd2;
    cmd2.set_deck(gOppDeck);
    mLocalClients[1]->sendGameCommand(cmd2);

    CommandReadyToStart readyCmd;
    readyCmd.set_ready(true);
    mLocalClients[0]->sendGameCommand(readyCmd);
    mLocalClients[1]->sendGameCommand(readyCmd);
}

void Game::processGameEvent(const std::shared_ptr<GameEvent> event) {
    mEventQueue.push_back(event);
    processGameEventFromQueue();
}

void Game::processGameEventFromQueue() {
    if (mActionInProgress || mUiActionInProgress || mEventQueue.empty())
        return;

    if (!mPlayer)
        return;

    try {
        auto event = mEventQueue.front();
        mEventQueue.pop_front();
        if (mPlayer->id() == event->player_id())
            mPlayer->processGameEvent(event);
        else if (mOpponent)
            mOpponent->processGameEvent(event);

        if (!mEventQueue.empty())
            QMetaObject::invokeMethod(this, "processGameEventFromQueue", Qt::QueuedConnection);
    } catch (const std::exception& ex) {
        emit endGamePrematurely(QString("Error occurred: ") + ex.what(), "start");
    }
}

void Game::cardMoveFinished() {
    sender()->deleteLater();
}

void Game::processLobbyEventLocal(const std::shared_ptr<LobbyEvent> event) {
    if (event->event().Is<EventGameJoined>()) {
        EventGameJoined ev;
        event->event().UnpackTo(&ev);
        localGameCreated(ev);
    }
}

void Game::processLobbyEventLocal2ndPlayer(const std::shared_ptr<LobbyEvent> event) {
    if (event->event().Is<EventGameJoined>()) {
        EventGameJoined ev;
        event->event().UnpackTo(&ev);
        localOpponentJoined(ev);
    }
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

void Game::sendTakeDamageCommand() {
    mPlayer->sendTakeDamageCommand();
}

void Game::sendEncoreCommand() {
    mPlayer->sendEncoreCommand();
}

void Game::sendEndTurn() {
    mPlayer->sendEndTurnCommand();
}

void Game::quitGame() {
    if (mIsLocal) {
        QMetaObject::invokeMethod(this, "quitGameQml");
    } else {
        mClient->sendGameCommand(CommandLeaveGame());
        emit gameEnded();
    }
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
    if (!mIsLocal)
        return mClient;
    return mLocalClients.at(playerId - 1).get();
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

void Game::endCounterStep() {
    QMetaObject::invokeMethod(this, "endCounterStep");
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

    if (mIsLocal) {
        disconnect(mLocalClients.front().get(), SIGNAL(gameEventReceived(const std::shared_ptr<GameEvent>)),
                   this, SLOT(processGameEvent(const std::shared_ptr<GameEvent>)));
        disconnect(mLocalClients.back().get(), SIGNAL(gameEventReceived(const std::shared_ptr<GameEvent>)),
                   this, SLOT(processGameEventByOpponent(const std::shared_ptr<GameEvent>)));
    } else {
        disconnect(mClient, &Client::gameEventReceived, this, &Game::processGameEvent);
    }
    mEventQueue.clear();
}

void Game::playerLeft() {
    hideText();
    emit endGamePrematurely("Opponent left the game", "lobby");
}

void Game::setPlayerDeck(const DeckList &deck) {
    mPlayer->setDeck(deck);
}



void Game::testAction() {
    mPlayer->testAction();
    //mOpponent->testAction();
}


#include "cardModel.h"
void Game::processGameEventByOpponent(const std::shared_ptr<GameEvent> event) {
    if (mOpponent->id() != event->player_id()) {
        if (event->event().Is<EventMoveDestinationIndexChoice>()) {
            CommandChoice cmd;
            cmd.set_choice(2);
            QTimer::singleShot(2000, this, [this, cmd]() { sendGameCommand(cmd, mOpponent->id()); });
        }
        return;
    }

    static CardModel hand;
    if (event->event().Is<EventInitialHand>()) {
        EventInitialHand ev;
        event->event().UnpackTo(&ev);
        for (int i = 0; i < ev.cards_size(); ++i) {
            mOpponent->zone("deck")->model().removeCard(mOpponent->zone("deck")->model().count());
            hand.addCard(ev.cards(i).id(), ev.cards(i).code(), mOpponent->zone("hand"));
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
        sendGameCommand(CommandClimaxPhase(), mOpponent->id());
    } else if (event->event().Is<EventClimaxPhase>()) {
        sendGameCommand(CommandAttackPhase(), mOpponent->id());
    } else if (event->event().Is<EventPlayCard>()) {
        EventPlayCard ev;
        event->event().UnpackTo(&ev);
        hand.removeCard(ev.hand_pos());
        sendGameCommand(CommandAttackPhase(), mOpponent->id());
        //if (mOpponent->playCards(hand))
        //    sendGameCommand(CommandAttackPhase(), mOpponent->id());
    } else if (event->event().Is<EventAttackDeclarationStep>()) {
        //timing problems
        mOpponent->attackWithAll();

        sendGameCommand(CommandEncoreStep(), mOpponent->id());
    } else if (event->event().Is<EventCounterStep>()) {
        CommandTakeDamage cmd;
        sendGameCommand(cmd, mOpponent->id());
    } else if (event->event().Is<EventLevelUp>()) {
        CommandLevelUp cmd;
        cmd.set_clock_pos(0);
        sendGameCommand(cmd, mOpponent->id());
    } else if (event->event().Is<EventEncoreStep>()) {
        CommandEndTurn cmd;
        sendGameCommand(cmd, mOpponent->id());
    } else if (event->event().Is<EventMoveCard>()) {
        EventMoveCard ev;
        event->event().UnpackTo(&ev);
        if (ev.start_zone() == "deck" && ev.target_zone() == "hand") {
            hand.addCard(ev.card_id(), ev.code(), mOpponent->zone("hand"));
        }
    } else if (event->event().Is<EventDiscardDownTo7>()) {
        CommandMoveCard cmd;
        cmd.set_start_pos(0);
        cmd.set_start_zone("hand");
        cmd.set_target_zone("wr");
        sendGameCommand(cmd, mOpponent->id());
    } else if (event->event().Is<EventEffectChoice>()) {
        CommandChoice cmd;
        cmd.set_choice(2);
        sendGameCommand(cmd, mOpponent->id());
    } else if (event->event().Is<EventChooseCard>()) {
        CommandChooseCard cmd;
        cmd.set_zone("stage");
        cmd.set_owner(ProtoOwner::ProtoOpponent);
        cmd.add_positions(1);
        cmd.add_positions(2);
        sendGameCommand(cmd, mOpponent->id());
    }
}
