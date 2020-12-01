#pragma once

#include <string>
#include <unordered_map>

#include "deckList.h"
#include "serverCardZone.h"

class GameCommand;
class ServerGame;
class ServerProtocolHandler;

class ServerPlayer
{
    ServerGame *mGame;
    ServerProtocolHandler *mClient;
    size_t mId;
    bool mReady;
    std::unique_ptr<DeckList> mDeck;
    std::unordered_map<std::string, std::unique_ptr<ServerCardZone>> mZones;

public:
    ServerPlayer(ServerGame *game, ServerProtocolHandler *client, size_t id);

    size_t id() const { return mId; }
    bool ready() const { return mReady; }

    void processGameCommand(GameCommand &cmd);
    void addDeck(const std::string &deck);
    void addZone(std::string_view name);
    void setReady(bool ready) { mReady = ready; }
    void setupZones();
};
