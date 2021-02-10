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

void Player::revealTopDeck(const EventRevealTopDeck &event) {
    zone("view")->visualItem()->setProperty("mViewMode", Game::RevealMode);

    QString code = QString::fromStdString(event.code());
    mGame->pause(400);
    createMovingCard(code, "deck", 0, "view", 0, false, true, true);
}

void Player::activateAbilities(const EventAbilityActivated &event) {
    stopUiInteractions();
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
                a.text = QString::fromStdString(printAbility(decodeAbility(abuf)));
            }
        } else {
            a.code = protoa.cardcode();
            if (protoa.type() == ProtoAbilityType::ProtoCard)
                a.text = zone(a.zone)->cards()[a.cardId].text(a.abilityId);
        }
        if (protoa.type() == ProtoAbilityType::ProtoClimaxTrigger)
            a.text = QString::fromStdString(printAbility(globalAbility(static_cast<TriggerIcon>(protoa.abilityid()))));

        if (event.abilities_size() > 1) {
            if (!mOpponent)
                a.playBtnActive = true;
            a.active = false;
        } else {
            a.active = true;
        }
        mAbilityList->addAbility(a);
    }
    if (mAbilityList->count())
        mGame->pause(250);
}

void Player::playAbility(int index) {
    const auto &ab = mAbilityList->ability(index);
    if (ab.active) {
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
    }
}

void Player::abilityResolved() {
    mGame->pause(500);
    mAbilityList->removeActiveAbility();
    auto view = zone("view");
    if (view->model().count())
        QMetaObject::invokeMethod(view->visualItem(), "clear");
        //view->model().clear();
    if (!mAbilityList->count()) {
        restoreUiState();
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

void Player::cancelAbility(int) {
    doneChoosing();
    sendGameCommand(CommandCancelEffect());
}

void Player::chooseCard(int, QString qzone, bool opponent) {
    if (!mAbilityList->count())
        return;
    int activeId = mAbilityList->activeId();
    auto &effect = mAbilityList->ability(activeId).effect;
    if (!std::holds_alternative<asn::ChooseCard>(effect))
        return;

    auto &ef = std::get<asn::ChooseCard>(effect);
    assert(ef.targets.size() == 1);
    if (ef.targets[0].type != asn::TargetType::SpecificCards)
        return;

    CardZone *pzone;
    if (!opponent)
        pzone = zone(qzone.toStdString());
    else
        pzone = mGame->opponent()->zone(qzone.toStdString());

    int num = pzone->numOfSelectedCards();

    if (ef.targets[0].targetSpecification->number.value != num)
        return;

    mAbilityList->activateCancel(activeId, false);

    CommandChooseCard cmd;
    cmd.set_zone(qzone.toStdString());
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
    CommandChoice cmd;
    cmd.set_choice(index);
    sendGameCommand(cmd);
}
