#include "serverPlayer.h"

#include "gameCommand.pb.h"

#include "serverCardZone.h"
#include "serverGame.h"

ServerPlayer::ServerPlayer(ServerGame *game, ServerProtocolHandler *client, size_t id)
    : mGame(game), mClient(client), mId(id), mReady(false) { }

void ServerPlayer::processGameCommand(GameCommand &cmd) {
    if (cmd.command().Is<CommandSetDeck>()) {
        CommandSetDeck setDeckCmd;
        cmd.command().UnpackTo(&setDeckCmd);
        addDeck(setDeckCmd.deck());
    } else if (cmd.command().Is<CommandReadyToStart>()) {
        CommandReadyToStart readyCmd;
        cmd.command().UnpackTo(&readyCmd);
        setReady(readyCmd.ready());
        mGame->startGame();
    }
}

void ServerPlayer::addDeck(const std::string &deck) {
    mDeck = std::make_unique<DeckList>(deck);
}

void ServerPlayer::addZone(std::string_view name) {
    mZones.emplace(name, std::make_unique<ServerCardZone>(this, name));
}

void ServerPlayer::setupZones() {
    addZone("deck");
    addZone("wr");
    addZone("hand");
    addZone("clock");
    addZone("stock");
    addZone("memory");
    addZone("climax");
}
