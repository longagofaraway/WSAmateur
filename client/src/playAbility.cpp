#include "player.h"

#include "abilities.h"
#include "abilityUtils.h"
#include "cardDatabase.h"
#include "codecs/decode.h"
#include "codecs/print.h"
#include "game.h"
#include "globalAbilities/globalAbilities.h"
#include "hand.h"
#include "stage.h"

namespace {
asn::ChooseCard decodeChooseCard(const std::string &buf) {
    std::vector<uint8_t> binbuf(buf.begin(), buf.end());
    auto it = binbuf.begin();
    return ::decodeChooseCard(it, binbuf.end());
}
asn::MoveCard decodeMoveCard(const std::string &buf) {
    std::vector<uint8_t> binbuf(buf.begin(), buf.end());
    auto it = binbuf.begin();
    return ::decodeMoveCard(it, binbuf.end());
}
asn::DrawCard decodeDrawCard(const std::string &buf) {
    std::vector<uint8_t> binbuf(buf.begin(), buf.end());
    auto it = binbuf.begin();
    return ::decodeDrawCard(it, binbuf.end());
}
asn::SearchCard decodeSearchCard(const std::string &buf) {
    std::vector<uint8_t> binbuf(buf.begin(), buf.end());
    auto it = binbuf.begin();
    return ::decodeSearchCard(it, binbuf.end());
}
asn::AbilityGain decodeAbilityGain(const std::string &buf) {
    std::vector<uint8_t> binbuf(buf.begin(), buf.end());
    auto it = binbuf.begin();
    return ::decodeAbilityGain(it, binbuf.end());
}
asn::Ability decodeAbility(const std::string &buf) {
    std::vector<uint8_t> binbuf(buf.begin(), buf.end());
    auto it = binbuf.begin();
    return ::decodeAbility(it, binbuf.end());
}

void highlightAllCards(CardZone *zone, bool highlight) {
    for (int i = 0; i < zone->model().count(); ++i)
        zone->model().setGlow(i, highlight);
}
void selectAllCards(CardZone *zone, bool select) {
    for (int i = 0; i < zone->model().count(); ++i)
        zone->model().setSelected(i, select);
}

int highlightEligibleCards(CardZone *zone, const std::vector<asn::CardSpecifier> &specs) {
    int eligibleCount = 0;
    const auto &cards = zone->cards();
    highlightAllCards(zone, false);
    selectAllCards(zone, false);

    for (int i = 0; i < zone->model().count(); ++i) {
        if (!cards[i].cardPresent())
            continue;

        if (checkCard(specs, cards[i])) {
            zone->model().setGlow(i, true);
            eligibleCount++;
        }
    }

    return eligibleCount;
}
}

void Player::processChooseCard(const EventChooseCard &event) {
    if (mOpponent)
        return;

    auto effect = decodeChooseCard(event.effect());
    if (effect.placeType != asn::PlaceType::SpecificPlace ||
        effect.targets.size() != 1 ||
        effect.targets[0].type != asn::TargetType::SpecificCards ||
        effect.targets[0].targetSpecification->number.mod != asn::NumModifier::ExactMatch ||
        effect.targets[0].targetSpecification->number.value != 1) {
        assert(false);
        return;
    }

    const auto &specs = effect.targets[0].targetSpecification->cards.cardSpecifiers;
    int eligibleCount = 0;
    if (effect.place->owner == asn::Player::Player || effect.place->owner == asn::Player::Both) {
        auto from = zone(asnZoneToString(effect.place->zone));
        eligibleCount += highlightEligibleCards(from, specs);
    }
    if (effect.place->owner == asn::Player::Opponent || effect.place->owner == asn::Player::Both) {
        auto from = mGame->opponent()->zone(asnZoneToString(effect.place->zone));
        eligibleCount += highlightEligibleCards(from, specs);
    }
    if (eligibleCount) {
        mAbilityList->ability(mAbilityList->activeId()).effect = effect;
        if (effect.place->zone == asn::Zone::WaitingRoom &&
            effect.place->owner == asn::Player::Player)
            QMetaObject::invokeMethod(zone("wr")->visualItem(), "openView", Q_ARG(QVariant, true));
        if (!event.mandatory())
            mAbilityList->activateCancel(mAbilityList->activeId(), true);
    } else {
        mGame->pause(800);
        sendGameCommand(CommandCancelEffect());
    }
}

void Player::processSearchCard(const EventSearchCard &event) {
    auto effect = decodeSearchCard(event.effect());
    assert(effect.place.zone == asn::Zone::Deck);
    assert(effect.targets.size() == 1);
    assert(effect.targets[0].cards.size() == 1);
    mDeckView->setCards(event.codes());
    const auto &specs = effect.targets[0].cards[0].cardSpecifiers;
    int eligibleCount = highlightEligibleCards(mDeckView, specs);

    if (eligibleCount) {
        mAbilityList->ability(mAbilityList->activeId()).effect = effect;
        if (effect.targets[0].number.mod == asn::NumModifier::UpTo)
            mAbilityList->activateCancel(mAbilityList->activeId(), true);
    } else {
        mGame->pause(800);
        sendGameCommand(CommandCancelEffect());
    }
}

void Player::processMoveChoice(const EventMoveChoice &event) {
    if (mOpponent)
        return;

    auto effect = decodeMoveCard(event.effect());
    assert(effect.executor == asn::Player::Player);

    if (effect.to.size() > 1) {
        std::vector<QString> data;
        for (const auto &to: effect.to) {
            assert(to.owner == asn::Player::Player);
            assert(to.pos == asn::Position::NotSpecified);
            data.push_back(asnZoneToReadableString(to.zone));
        }

        mChoiceDialog->setData("Choose where to put the card", data);
        return;
    }

    auto header = printMoveCard(effect);
    header[0] = std::toupper(header[0]);
    header.push_back('?');
    if (!event.mandatory()) {
        std::vector<QString> data { "Yes", "No" };
        mChoiceDialog->setData(QString::fromStdString(header), data);
    }
}

void Player::processDrawChoice(const EventDrawChoice &event) {
    if (mOpponent)
        return;

    auto effect = decodeDrawCard(event.effect());

    auto header = printDrawCard(effect);
    header[0] = std::toupper(header[0]);
    header.push_back('?');
    if (!event.mandatory()) {
        std::vector<QString> data { "Yes", "No" };
        mChoiceDialog->setData(QString::fromStdString(header), data);
    }
}

void Player::processAbilityChoice(const EventAbilityChoice &event) {
    if (mOpponent)
        return;

    auto effect = decodeAbilityGain(event.effect());
    if (static_cast<size_t>(effect.number) < effect.abilities.size()) {
        std::vector<QString> data;
        for (const auto &a: effect.abilities)
            data.push_back(QString::fromStdString(printAbility(a)));
        mChoiceDialog->setData("Choose ability", data);
        mAbilityList->ability(mAbilityList->activeId()).effect = effect;
    }
}

void Player::processAbilityGain(const EventAbilityGain &event) {
    auto ability = decodeAbility(event.ability());
    auto pzone = zone(event.zone());
    auto &card = pzone->cards()[event.cardid()];
    if (!card.cardPresent())
        return;
    card.addAbility(ability);
}

void Player::revealTopDeck(const EventRevealTopDeck &event) {
    zone("view")->visualItem()->setProperty("mViewMode", Game::RevealMode);

    QString code = QString::fromStdString(event.code());
    mGame->pause(400);
    createMovingCard(code, "deck", 0, "view", 0, false, true, true);
}

void Player::activateAbilities(const EventAbilityActivated &event) {
    stopUiInteractions();
    int playableCount = 0;
    for (int i = 0; i < event.abilities_size(); ++i) {
        auto protoa = event.abilities(i);
        auto zoneptr = zone(protoa.zone());
        if (!zoneptr)
            return;
        const auto &cards = zoneptr->cards();
        if (protoa.cardid() < 0)
            return;

        bool nocard = false;
        if (static_cast<size_t>(protoa.cardid()) > cards.size() ||
            cards[protoa.cardid()].code() != protoa.cardcode())
            nocard = true;

        asn::Ability ability;
        ActivatedAbility a;
        a.uniqueId = protoa.uniqueid();
        a.type = protoa.type();
        a.zone = protoa.zone();
        a.cardId = protoa.cardid();
        a.abilityId = protoa.abilityid();
        if (nocard) {
            auto cardInfo = CardDatabase::get().getCard(protoa.cardcode());
            if (!cardInfo)
                return;

            a.code = cardInfo->code();
            if (protoa.type() == ProtoAbilityType::ProtoCard) {
                const auto &abuf = cardInfo->abilities()[a.abilityId];
                a.ability = decodeAbility(abuf);
                a.text = QString::fromStdString(printAbility(ability));
            }
        } else {
            a.code = protoa.cardcode();
            if (protoa.type() == ProtoAbilityType::ProtoCard) {
                const auto &card = zone(a.zone)->cards()[a.cardId];
                a.ability = card.ability(a.abilityId);
                a.text = card.text(a.abilityId);
            }
        }
        if (protoa.type() == ProtoAbilityType::ProtoClimaxTrigger)
            a.text = QString::fromStdString(printAbility(globalAbility(static_cast<TriggerIcon>(protoa.abilityid()))));

        if (event.abilities_size() > 1) {
            if (canPlay(a.ability))
                ++playableCount;
            a.active = false;
        } else {
            a.active = true;
        }
        mAbilityList->addAbility(a);
    }
    if (!mOpponent) {
        if (playableCount == 1) {
            // if there's just one ability that can be played, make it active
            for (int i = 0; i < mAbilityList->count(); ++i)
                if (canPlay(mAbilityList->ability(i).ability))
                    mAbilityList->setActive(i, true);
        } else if (playableCount > 1) {
            // otherwise show 'Play' button on playable abilities
            for (int i = 0; i < mAbilityList->count(); ++i)
                if (canPlay(mAbilityList->ability(i).ability))
                    mAbilityList->activatePlay(i, true);
        }
    }
    if (mAbilityList->count())
        mGame->pause(250);
}

void Player::playAbility(int index) {
    const auto &ab = mAbilityList->ability(index);
    if (ab.active) {
        mAbilityList->activatePlay(index, false);
        mAbilityList->activateCancel(index, false);
        sendGameCommand(CommandPlayEffect());
    } else {
        mAbilityList->setActive(index, true);
        for (int i = 0; i < mAbilityList->count(); ++i) {
            mAbilityList->activatePlay(i, false);
        }
        CommandPlayAbility cmd;
        cmd.set_uniqueid(ab.uniqueId);
        sendGameCommand(cmd);
    }
}

void Player::doneChoosing() {
    auto &effect = mAbilityList->ability(mAbilityList->activeId()).effect;
    if (std::holds_alternative<asn::ChooseCard>(effect)) {
        auto &ef = std::get<asn::ChooseCard>(effect);
        assert(ef.placeType == asn::PlaceType::SpecificPlace);
        if (ef.place->zone == asn::Zone::WaitingRoom) {
            QMetaObject::invokeMethod(zone("wr")->visualItem(), "openView", Q_ARG(QVariant, false));
        }
        if (ef.place->owner == asn::Player::Player || ef.place->owner == asn::Player::Both) {
            auto from = zone(asnZoneToString(ef.place->zone));
            highlightAllCards(from, false);
            selectAllCards(from, false);
        }
        if (ef.place->owner == asn::Player::Opponent || ef.place->owner == asn::Player::Both) {
            auto from = mGame->opponent()->zone(asnZoneToString(ef.place->zone));
            highlightAllCards(from, false);
            selectAllCards(from, false);
        }
    } else if (std::holds_alternative<asn::SearchCard>(effect)) {
        mDeckView->hide();
    }
}

void Player::abilityResolved() {
    mGame->pause(500);
    mAbilityList->removeActiveAbility();
    auto view = zone("view");
    if (view->model().count())
        QMetaObject::invokeMethod(view->visualItem(), "clear");
    if (!mAbilityList->count()) {
        restoreUiState();
    } else {
        int playableCount = 0;
        int playableId = 0;
        for (int i = 0; i < mAbilityList->count(); ++i) {
            if (canPlay(mAbilityList->ability(i).ability)) {
                ++playableCount;
                playableId = i;
            }
        }
        if (playableCount == 1) {
            mAbilityList->setActive(playableId, true);
        } else if (playableCount > 1) {
            for (int i = 0; i < mAbilityList->count(); ++i)
                if (canPlay(mAbilityList->ability(i).ability))
                    mAbilityList->activatePlay(i, false);
        }
    }
}

void Player::stopUiInteractions() {
    if (mOpponent)
        return;

    switch (mGame->phase()) {
    case asn::Phase::MainPhase:
        mHand->endMainPhase();
        mStage->endMainPhase();
        mGame->pauseMainPhase();
        break;
    default:
        break;
    }
}

void Player::restoreUiState() {
    if (mOpponent)
        return;

    switch (mGame->phase()) {
    case asn::Phase::MainPhase: {
        const auto &cards = mHand->cards();
        for (int i = 0; i < mHand->model().count(); ++i) {
            if (canPlay(cards[i]))
                mHand->model().setGlow(i, true);
        }

        mHand->mainPhase();
        mStage->mainPhase();
        mGame->mainPhase();
        break;
    }
    case asn::Phase::AttackPhase: {
        Player *activePlayer = mActivePlayer ? this : mGame->opponent();
        auto pzone = activePlayer->zone("stage");
        const auto &card = pzone->cards()[activePlayer->mAttackingId];
        if (card.cardPresent() && card.state() == StateRested)
            pzone->model().setSelected(activePlayer->mAttackingId, true);
        break;
    }
    default:
        break;
    }
}

void Player::makeAbilityActive(const EventPlayAbility &event) {
    mAbilityList->setActiveByUniqueId(event.uniqueid(), true);
}

void Player::conditionNotMet() {
    if (zone("view")->model().count())
        mGame->pause(900);
}

void Player::payCostChoice() {
    if (mOpponent)
        return;

    mAbilityList->activatePlay(mAbilityList->activeId(), true, "Pay cost");
    mAbilityList->activateCancel(mAbilityList->activeId(), true);
}

void Player::cancelAbility(int) {
    doneChoosing();
    sendGameCommand(CommandCancelEffect());
}

void Player::chooseCard(int, QString qzone, bool opponent) {
    if (!mAbilityList->count())
        return;

    CardZone *pzone;
    if (!opponent)
        pzone = zone(qzone.toStdString());
    else
        pzone = mGame->opponent()->zone(qzone.toStdString());

    int selected = pzone->numOfSelectedCards();
    int highlighted = pzone->numOfHighlightedCards();

    int activeId = mAbilityList->activeId();
    auto &effect = mAbilityList->ability(activeId).effect;
    asn::Number *number;
    if (std::holds_alternative<asn::ChooseCard>(effect)) {
        auto &ef = std::get<asn::ChooseCard>(effect);
        assert(ef.targets.size() == 1);
        if (ef.targets[0].type != asn::TargetType::SpecificCards)
            return;

        number = &ef.targets[0].targetSpecification->number;
    } else if (std::holds_alternative<asn::SearchCard>(effect)) {
        auto &ef = std::get<asn::SearchCard>(effect);
        assert(ef.targets.size() == 1);
        number = &ef.targets[0].number;
    }

    if (number->value != selected && selected < highlighted) {
        if (number->mod == asn::NumModifier::UpTo)
            mAbilityList->activatePlay(activeId, true, "Choose");
        return;
    }

    mAbilityList->activatePlay(activeId, false);
    mAbilityList->activateCancel(activeId, false);

    CommandChooseCard cmd;
    std::string cmdZone = qzone.toStdString();
    if (cmdZone == "deckView")
        cmdZone = "deck";
    cmd.set_zone(cmdZone);
    cmd.set_owner(opponent ? ProtoOpponent : ProtoPlayer);
    const auto &cards = pzone->cards();
    for (int i = 0; i < static_cast<int>(cards.size()); ++i) {
        if (cards[i].selected()) {
            cmd.add_ids(i);
        }
    }
    sendGameCommand(cmd);
    doneChoosing();
}

void Player::sendChoice(int index) {
    int activeId = mAbilityList->activeId();
    auto &effect = mAbilityList->ability(activeId).effect;
    if (std::holds_alternative<asn::AbilityGain>(effect)) {
        auto &ef = std::get<asn::AbilityGain>(effect);
        if (static_cast<size_t>(index) > ef.abilities.size())
            return;
        if (ef.target.type == asn::TargetType::ThisCard) {
            auto pzone = zone(mAbilityList->ability(activeId).zone);
            auto &card = pzone->cards()[mAbilityList->ability(activeId).cardId];
            if (!card.cardPresent())
                return;

        }
    }
    CommandChoice cmd;
    cmd.set_choice(index);
    sendGameCommand(cmd);
}

bool Player::canPay(const asn::CostItem &c) const {
    if (c.type == asn::CostType::Stock) {
        const auto &item = std::get<asn::StockCost>(c.costItem);
        if (item.value > zone("stock")->model().count())
            return false;
        return true;
    } else {
        const auto &item = std::get<asn::Effect>(c.costItem);
        if (item.type == asn::EffectType::MoveCard) {
            const auto &e = std::get<asn::MoveCard>(item.effect);
            if (e.from.zone == asn::Zone::Hand &&
                e.target.targetSpecification->number.value > zone("hand")->model().count())
                return false;
        }
        return true;
    }
}

bool Player::canPlay(const asn::Ability &a) const {
    if (a.type == asn::AbilityType::Auto) {
        const auto &aa = std::get<asn::AutoAbility>(a.ability);
        if (!aa.cost)
            return true;
        for (const auto &costItem: aa.cost->items)
            if (!canPay(costItem))
                return false;
        return true;
    }

    return false;
}
