#include "game.h"

#include "localClientConnection.h"
#include "localServer.h"

#include "lobbyCommand.pb.h"

Game::Game() {
    qRegisterMetaType<std::shared_ptr<EventGameJoined>>("std::shared_ptr<EventGameJoined>");
}

Game::~Game() {
    mClientThread->quit();
    mClientThread->wait();
}

void Game::componentComplete() {
    QQuickItem::componentComplete();

    startLocalGame();
}

void Game::startLocalGame() {
    mClientThread = std::make_unique<QThread>();
    mLocalServer = std::make_unique<LocalServer>();
    mLocalServer->moveToThread(mClientThread.get());

    addClient();
    addClient();

    mClientThread->start();

    connect(mClients.front().get(), SIGNAL(gameJoinedEventReceived(const std::shared_ptr<EventGameJoined>)),
            this, SLOT(localGameJoined(const std::shared_ptr<EventGameJoined>)));

    CommandCreateGame command;
    command.set_description("hah");
    mClients.front()->sendLobbyCommand(command);
}

void Game::addClient() {
    auto serverConnection = mLocalServer->newConnection();
    auto localConnection = std::make_shared<LocalClientConnection>(serverConnection.get());
    localConnection->moveToThread(mClientThread.get());

    auto client = std::make_unique<Client>(localConnection);
    client->moveToThread(mClientThread.get());
    mClients.emplace_back(std::move(client));
}

void Game::localGameJoined(const std::shared_ptr<EventGameJoined> event) {
    mPlayer = std::make_unique<Player>(this, false);
    mOpponent = std::make_unique<Player>(this, true);
}

QQmlEngine* Game::engine() const { return qmlEngine(parentItem()); }
QQmlContext* Game::context() const { return qmlContext(parentItem()); }
