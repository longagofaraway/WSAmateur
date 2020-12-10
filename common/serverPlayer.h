#pragma once

#include <string>
#include <unordered_map>

#include <google/protobuf/message.h>

#include "commands.h"
#include "deckList.h"
#include "serverCardZone.h"

class GameCommand;
class ServerGame;
class ServerProtocolHandler;
class CommandMulligan;

class ServerPlayer
{
    ServerGame *mGame;
    ServerProtocolHandler *mClient;
    size_t mId;
    bool mReady = false;
    bool mMulliganFinished = false;
    std::unique_ptr<DeckList> mDeck;
    std::unordered_map<std::string_view, std::unique_ptr<ServerCardZone>> mZones;
    std::vector<ExpectedCommand> mExpectedCommands;

public:
    ServerPlayer(ServerGame *game, ServerProtocolHandler *client, size_t id);

    size_t id() const { return mId; }
    bool ready() const { return mReady; }
    bool mulliganFinished() const { return mMulliganFinished; }

    void processGameCommand(GameCommand &cmd);
    void sendGameEvent(const ::google::protobuf::Message &event, size_t playerId = 0);
    void addDeck(const std::string &deck);
    DeckList* deck() { return mDeck.get(); }
    ServerCardZone* addZone(std::string_view name);
    ServerCardZone* zone(std::string_view name);
    void setReady(bool ready) { mReady = ready; }
    void setupZones();
    void startGame();

    void addExpectedCommand(const std::string &command);
    bool expectsCommand(const GameCommand &command);

    void dealStartingHand();
    void mulligan(const CommandMulligan &cmd);
};
