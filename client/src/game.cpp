#include "game.h"

#include "localClientConnection.h"
#include "localServer.h"

#include "gameCommand.pb.h"
#include "lobbyCommand.pb.h"
#include "lobbyEvent.pb.h"

Game::Game() {
    qRegisterMetaType<std::shared_ptr<EventGameJoined>>("std::shared_ptr<EventGameJoined>");
}

Game::~Game() {
    mClientThread.quit();
    mClientThread.wait();
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

    connect(mClients.front().get(), SIGNAL(gameJoinedEventReceived(const std::shared_ptr<EventGameJoined>)),
            this, SLOT(localGameCreated(const std::shared_ptr<EventGameJoined>)));

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

QQmlEngine* Game::engine() const { return qmlEngine(parentItem()); }
QQmlContext* Game::context() const { return qmlContext(parentItem()); }
