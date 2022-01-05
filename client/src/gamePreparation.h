#pragma once

#include <memory>

#include <QQuickItem>

#include "deckList.h"

class Game;
class Client;
class ImageLoader;

class GameEvent;

class GamePreparation : public QQuickItem
{
    Q_OBJECT
private:
    ImageLoader *imageLoader;
    Client *client;
    Game *game;
    DeckList chosenDeck;
    bool ready = false;
    bool oppImagesLoaded = false;

public:
    GamePreparation();

    Q_INVOKABLE void init(Game *game_);
    Q_INVOKABLE void sendDeck(QString deckName);
    Q_INVOKABLE void setReady();
    Q_INVOKABLE void oppDeckImagesDownloaded();

signals:
    void sigSetOpponentDeck(QString xmlDeck);

private slots:
    void processGameEvent(const std::shared_ptr<GameEvent> event);
    void setOpponentDeck(const std::string &xmlDeck);

private:
    void sendReady();

protected:
    void componentComplete() override;
};

