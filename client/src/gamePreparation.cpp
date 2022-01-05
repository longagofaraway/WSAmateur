#include "gamePreparation.h"

#include "gameCommand.pb.h"

#include "client.h"
#include "deckList.h"
#include "deckUtils.h"
#include "game.h"
#include "imageLoader.h"

GamePreparation::GamePreparation() {}

void GamePreparation::init(Game *game_) {
    game = game_;
    client = game->client();
    connect(client, &Client::gameEventReceived, this, &GamePreparation::processGameEvent);
    connect(game, &Game::opponentDeckSet, this, &GamePreparation::setOpponentDeck);

    game->preGameLoaded();
}

void GamePreparation::sendDeck(QString deckName) {
    auto deck = getDeckByName(deckName);
    CommandSetDeck command;
    command.set_deck(deck->toXml().toStdString());
    client->sendGameCommand(command);
}

void GamePreparation::setReady() {
    ready = true;
    if (oppImagesLoaded) {
        sendReady();
        deleteLater();
    }
}

void GamePreparation::oppDeckImagesDownloaded() {
    oppImagesLoaded = true;
    if (ready) {
        sendReady();
        deleteLater();
    }
}

void GamePreparation::processGameEvent(const std::shared_ptr<GameEvent> event) {

}

void GamePreparation::setOpponentDeck(const std::string &xmlDeck) {
    if (xmlDeck.empty())
        return;

    emit sigSetOpponentDeck(QString::fromStdString(xmlDeck));
}

void GamePreparation::sendReady() {
    CommandReadyToStart command;
    command.set_ready(true);
    client->sendGameCommand(command);
}

void GamePreparation::componentComplete() {
    QQuickItem::componentComplete();
}


