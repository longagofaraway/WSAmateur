#include "gamePreparation.h"

#include "gameCommand.pb.h"

#include "client.h"
#include "deckList.h"
#include "deckUtils.h"
#include "game.h"
#include "imageLoader.h"
#include "settingsManager.h"

GamePreparation::GamePreparation() {}

GamePreparation::~GamePreparation() {
    disconnect(game, &Game::opponentDeckSet, this, &GamePreparation::setOpponentDeck);
}

void GamePreparation::init(Game *game_) {
    game = game_;
    client = game->client();
    connect(game, &Game::opponentDeckSet, this, &GamePreparation::setOpponentDeck);

    game->preGameLoaded();

    auto &settingsManager = SettingsManager::get();
    const auto &deckName = settingsManager.localGameDeckName();
    if (settingsManager.localGameEnabled() && deckName.has_value()) {
        sendDeck(deckName.value());
        setReady();
    }
}

void GamePreparation::sendDeck(QString deckName) {
    auto deck = getDeckByName(deckName);
    if (!deck)
        return;
    game->setPlayerDeck(deck.value());

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
