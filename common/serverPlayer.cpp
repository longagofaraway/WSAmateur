#include "serverPlayer.h"

#include <algorithm>

#include "gameCommand.pb.h"
#include "gameEvent.pb.h"
#include "moveCommands.pb.h"
#include "moveEvents.pb.h"
#include "phaseCommand.pb.h"
#include "phaseEvent.pb.h"

#include "cardDatabase.h"
#include "serverCardZone.h"
#include "serverGame.h"
#include "serverProtocolHandler.h"

ServerPlayer::ServerPlayer(ServerGame *game, ServerProtocolHandler *client, int id)
    : mGame(game), mClient(client), mId(id) { }

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
    } else if (cmd.command().Is<CommandMulligan>()) {
        CommandMulligan mulliganCmd;
        cmd.command().UnpackTo(&mulliganCmd);
        mulligan(mulliganCmd);
    } else if (cmd.command().Is<CommandClockPhase>()) {
        CommandClockPhase clockCmd;
        cmd.command().UnpackTo(&clockCmd);
        processClockPhaseResult(clockCmd);
    } else if (cmd.command().Is<CommandPlayCard>()) {
        CommandPlayCard playCmd;
        cmd.command().UnpackTo(&playCmd);
        playCard(playCmd);
    } else if (cmd.command().Is<CommandSwitchStagePositions>()) {
        CommandSwitchStagePositions switchCmd;
        cmd.command().UnpackTo(&switchCmd);
        switchPositions(switchCmd);
    } else if (cmd.command().Is<CommandClimaxPhase>()) {
        climaxPhase();
    } else if (cmd.command().Is<CommandAttackPhase>()) {
        attackPhase();
    } else if (cmd.command().Is<CommandDeclareAttack>()) {
        CommandDeclareAttack declareAttackCmd;
        cmd.command().UnpackTo(&declareAttackCmd);
        declareAttack(declareAttackCmd);
    }
}

void ServerPlayer::sendGameEvent(const ::google::protobuf::Message &event, int playerId) {
    if (!playerId)
        playerId = mId;
    mClient->sendGameEvent(event, playerId);
}

void ServerPlayer::addDeck(const std::string &deck) {
    mDeck = std::make_unique<DeckList>(deck);
    mExpectedCommands.push_back(CommandReadyToStart::GetDescriptor()->name());
}

ServerCardZone* ServerPlayer::addZone(std::string_view name, ZoneType type) {
    return mZones.emplace(name, std::make_unique<ServerCardZone>(this, name, type)).first->second.get();
}

ServerCardZone* ServerPlayer::zone(std::string_view name) {
    if (!mZones.count(name))
        return nullptr;

    return mZones.at(name).get();
}

void ServerPlayer::setupZones() {
    auto deck = addZone("deck", ZoneType::HiddenZone);
    addZone("wr");
    addZone("hand", ZoneType::PrivateZone);
    addZone("clock");
    addZone("stock", ZoneType::HiddenZone);
    addZone("memory");
    addZone("climax");
    addZone("level");
    auto stage = addZone("stage");

    for (auto &card: mDeck->cards()) {
        auto cardInfo = CardDatabase::get().getCard(card.code);
        for (int i = 0; i < card.count; ++i)
            deck->addCard(cardInfo);
    }
    for (int i = 0; i < 5; ++i)
        stage->addCard(i);

    deck->shuffle();
}

void ServerPlayer::startGame() {
    mExpectedCommands.clear();
    mExpectedCommands.emplace_back(CommandMulligan::GetDescriptor()->name(), 1);
}

void ServerPlayer::startTurn() {
    EventStartTurn event;
    sendGameEvent(event);
    mGame->sendPublicEvent(event, mId);

    drawCards(1);

    EventClockPhase ev;
    sendGameEvent(ev);
    mGame->sendPublicEvent(ev, mId);

    addExpectedCommand(CommandClockPhase::GetDescriptor()->name());
}

void ServerPlayer::addExpectedCommand(const std::string &command) {
    mExpectedCommands.push_back(command);
}

void ServerPlayer::clearExpectedComands() {
    mExpectedCommands.clear();
}

bool ServerPlayer::expectsCommand(const GameCommand &command) {
    auto &fullCmdName = command.command().type_url();
    size_t pos = fullCmdName.find('/');
    if (pos == std::string::npos)
        throw std::runtime_error("error parsing GameCommand type");

    auto cmdName = fullCmdName.substr(pos + 1);

    auto it = std::find(mExpectedCommands.begin(), mExpectedCommands.end(), cmdName);
    if (it == mExpectedCommands.end())
        return false;

    if (it->commandArrived())
        mExpectedCommands.erase(it);

    return true;
}

void ServerPlayer::dealStartingHand() {
    auto deck = zone("deck");
    auto hand = zone("hand");

    EventInitialHand eventPrivate;
    eventPrivate.set_count(5);
    eventPrivate.set_firstturn(mStartingPlayer);
    EventInitialHand eventPublic(eventPrivate);
    for (int i = 0; i < 5; ++i) {
        auto card = deck->takeTopCard();
        auto code = eventPrivate.add_codes();
        *code = card->code();
        hand->addCard(std::move(card));
    }

    sendGameEvent(eventPrivate);
    mGame->sendPublicEvent(eventPublic, mId);
}

void ServerPlayer::mulligan(const CommandMulligan &cmd) {
    if (cmd.ids_size()) {
        std::vector<int> ids;
        for (int i = 0; i < cmd.ids_size(); ++i)
            ids.push_back(cmd.ids(i));

        moveCard("hand", ids, "wr");
        drawCards(ids.size());
    }
    mMulliganFinished = true;
    mGame->endMulligan();
}

void ServerPlayer::drawCards(int number) {
    auto deck = zone("deck");

    for (int i = 0; i < number; ++i) {
        moveCard("deck", { deck->count() - 1 }, "hand");
    }
}

void ServerPlayer::moveCard(std::string_view startZoneName,  const std::vector<int> &cardIds, std::string_view targetZoneName) {
    ServerCardZone *startZone = zone(startZoneName);
    if (!startZone)
        return;

    ServerCardZone *targetZone = zone(targetZoneName);
    if (!targetZone)
        return;

    auto sortedIds = cardIds;
    std::sort(sortedIds.begin(), sortedIds.end());
    for (int i = sortedIds.size(); i-- > 0;) {
        auto cardPtr = startZone->takeCard(sortedIds[i]);
        if (!cardPtr)
            return;

        ServerCard *card = targetZone->addCard(std::move(cardPtr));

        std::string code;
        EventMoveCard eventPublic;
        eventPublic.set_id(static_cast<google::protobuf::uint32>(sortedIds[i]));
        eventPublic.set_startzone(startZone->name());
        eventPublic.set_targetzone(targetZone->name());

        if (startZone->type() == ZoneType::HiddenZone && targetZone->type() == ZoneType::PublicZone)
            eventPublic.set_code(card->code());

        EventMoveCard eventPrivate(eventPublic);

        if (startZone->type() == ZoneType::HiddenZone && targetZone->type() == ZoneType::PrivateZone)
            eventPrivate.set_code(card->code());

        // revealing card from hand, opponent didn't see this card yet
        if (startZone->type() == ZoneType::PrivateZone && targetZone->type() == ZoneType::PublicZone)
            eventPublic.set_code(card->code());

        sendGameEvent(eventPrivate);
        mGame->sendPublicEvent(eventPublic, mId);
    }
}

void ServerPlayer::processClockPhaseResult(const CommandClockPhase &cmd) {
    if (cmd.count()) {
        moveCard("hand", { cmd.cardid() }, "clock");
        drawCards(2);
    }

    EventMainPhase ev;
    sendGameEvent(ev);
    mGame->sendPublicEvent(ev, mId);

    clearExpectedComands();
    addExpectedCommand(CommandPlayCard::GetDescriptor()->name());
    addExpectedCommand(CommandClimaxPhase::GetDescriptor()->name());
    addExpectedCommand(CommandAttackPhase::GetDescriptor()->name());
    addExpectedCommand(CommandSwitchStagePositions::GetDescriptor()->name());
    //TODO activate ability
}

void ServerPlayer::playCard(const CommandPlayCard &cmd) {
    auto *hand = zone("hand");
    auto cardPtr = hand->card(cmd.handid());
    if (!cardPtr)
        return;

    if (cardPtr->type() == CardType::Character)
        playCharacter(cmd);
    else if (cardPtr->type() == CardType::Climax)
        playClimax(cmd.handid());
}

void ServerPlayer::playCharacter(const CommandPlayCard &cmd) {
    ServerCardZone *hand = zone("hand");
    ServerCardZone *stage = zone("stage");
    if (cmd.stageid() >= stage->count())
        return;

    auto cardPtr = hand->card(cmd.handid());
    if (!cardPtr)
        return;

    if (!canPlay(cardPtr))
        return;

    auto card = hand->takeCard(cmd.handid());

    auto oldStageCard = stage->swapCards(std::move(card), cmd.stageid());
    auto cardInPlay = stage->card(cmd.stageid());

    EventPlayCard eventPublic;
    eventPublic.set_handid(cmd.handid());
    eventPublic.set_stageid(cmd.stageid());
    EventPlayCard eventPrivate(eventPublic);
    eventPublic.set_code(cardInPlay->code());

    sendGameEvent(eventPrivate);
    mGame->sendPublicEvent(eventPublic, mId);

    if (!oldStageCard)
        zone("wr")->addCard(std::move(oldStageCard));

    auto stock = zone("stock");
    for (int i = 0; i < cardInPlay->cost(); ++i)
        moveCard("stock", { stock->count() - 1 }, "wr");
}

void ServerPlayer::playClimax(int handIndex) {
    auto hand = zone("hand");
    auto climaxZone = zone("climax");

    auto card = hand->takeCard(handIndex);
    climaxZone->addCard(std::move(card));
    auto cardPtr = climaxZone->card(0);

    EventPlayCard eventPublic;
    eventPublic.set_handid(static_cast<google::protobuf::uint32>(handIndex));
    EventPlayCard eventPrivate(eventPublic);
    eventPublic.set_code(cardPtr->code());

    sendGameEvent(eventPrivate);
    mGame->sendPublicEvent(eventPublic, mId);

    //process climax effects
    attackPhase();
}

void ServerPlayer::switchPositions(const CommandSwitchStagePositions &cmd) {
    ServerCardZone *stage = zone("stage");
    if (cmd.stageidfrom() >= stage->count()
        || cmd.stageidto() >= stage->count())
        return;

    stage->swapCards(cmd.stageidfrom(), cmd.stageidto());
    EventSwitchStagePositions event;
    event.set_stageidfrom(cmd.stageidfrom());
    event.set_stageidto(cmd.stageidto());

    sendGameEvent(event);
    mGame->sendPublicEvent(event, mId);
}

bool ServerPlayer::canPlay(ServerCard *card) {
    if (mGame->phase() == Phase::Climax && card->type() != CardType::Climax)
        return false;
    /*if (card->level() > mLevel)
        return false;
    if (static_cast<int>(card->cost()) > zone("stock")->count())
        return false;
    if (card->level() > 0 || card->type() == CardType::Climax) {
        if (!zone("clock")->hasCardWithColor(card->color())
            && !zone("level")->hasCardWithColor(card->color()))
            return false;
    }*/
    return true;
}

void ServerPlayer::climaxPhase() {
    // check timing at start of climax phase
    mGame->setPhase(Phase::Climax);
    clearExpectedComands();
    addExpectedCommand(CommandPlayCard::GetDescriptor()->name());
    EventClimaxPhase event;
    sendGameEvent(event);
    mGame->sendPublicEvent(event, mId);
}

void ServerPlayer::attackPhase() {
    mGame->setPhase(Phase::Attack);
    clearExpectedComands();
    addExpectedCommand(CommandDeclareAttack::GetDescriptor()->name());
    EventAttackDeclarationStep event;
    sendGameEvent(event);
    mGame->sendPublicEvent(event, mId);
}

bool isCenterStagePosition(int pos) { return pos >= 3 ? false : true; }
void ServerPlayer::declareAttack(const CommandDeclareAttack &cmd) {
    auto stage = zone("stage");
    if (!isCenterStagePosition(cmd.stageid()))
        return;
    auto attCard = stage->card(cmd.stageid());
    if (attCard->state() != CardState::Standing)
        return;

    attCard->setState(CardState::Rested);

    AttackType type = cmd.attacktype();
    if (hasBattleOpponent(cmd.stageid()))
        type = AttackType::DirectAttack;

    EventDeclareAttack event;
    event.set_stageid(cmd.stageid());
    event.set_attacktype(type);
    sendGameEvent(event);
    mGame->sendPublicEvent(event, mId);
}

bool ServerPlayer::hasBattleOpponent(int pos) const {
    auto opponent = mGame->opponentOfPlayer(mId);
    if (!opponent)
        return false;

    if (opponent->zone("stage")->card(pos))
        return true;

    return false;
}
