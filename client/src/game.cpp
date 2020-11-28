#include "game.h"

#include "localConnection.h"
#include "localServer.h"


Game::~Game() {
    mClientThread->quit();
    mClientThread->wait();
}

void Game::componentComplete() {
    QQuickItem::componentComplete();

    startLocalGame();

    mPlayer = std::make_unique<Player>(this, false);
    mOpponent = std::make_unique<Player>(this, true);
}

void Game::startLocalGame() {
    mClientThread = std::make_unique<QThread>();
    mLocalServer = std::make_unique<LocalServer>();
    mLocalServer->moveToThread(mClientThread.get());

    addClient();
    addClient();

    mClientThread->start();
}

void Game::addClient() {
    auto serverConnection = mLocalServer->newConnection();
    auto localConnection = std::make_shared<LocalConnection>(serverConnection.get());
    localConnection->moveToThread(mClientThread.get());

    auto client = std::make_unique<Client>(localConnection);
    client->moveToThread(mClientThread.get());
    mClients.emplace_back(std::move(client));
}

QQmlEngine* Game::engine() const { return qmlEngine(parentItem()); }
QQmlContext* Game::context() const { return qmlContext(parentItem()); }
