#include "player.h"

#include "abilities.h"
#include "abilityUtils.h"
#include "cardDatabase.h"
#include "codecs/decode.h"
#include "game.h"
#include "globalAbilities/globalAbilities.h"

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

bool checkCard(const std::vector<asn::CardSpecifier> specs, const Card &card) {
    bool eligible = true;
    for (const auto &spec: specs) {
        switch (spec.type) {
        case asn::CardSpecifierType::CardType:
            if (std::get<CardType>(spec.specifier) != card.type())
                eligible = false;
            break;
        case asn::CardSpecifierType::TriggerIcon:
            bool found = false;
            for (auto triggerIcon: card.triggers()) {
                if (triggerIcon == std::get<TriggerIcon>(spec.specifier)) {
                    found = true;
                    break;
                }
            }
            if (!found)
                eligible = false;
            break;
        }
        if (!eligible)
            return false;
    }
    return true;
}
}

void Player::processChooseCard(const EventChooseCard &event) {
    if (mOpponent)
        return;

    auto effect = decodeChooseCard(event.effect());
    if (effect.placeType != asn::PlaceType::SpecificPlace ||
        effect.place->owner != asn::Owner::Player ||
        effect.targets.size() != 1 ||
        effect.targets[0].type != asn::TargetType::SpecificCards ||
        effect.targets[0].targetSpecification->number.mod != asn::NumModifier::ExactMatch ||
        effect.targets[0].targetSpecification->number.value != 1) {
        assert(false);
        return;
    }

    const auto &specs = effect.targets[0].targetSpecification->cards.cardSpecifiers;
    auto from = zone(asnZoneToString(effect.place->zone));
    const auto &cards = from->cards();
    int eligibleCount = 0;
    for (int i = 0; i < from->model().count(); ++i) {
        if (!cards[i].cardPresent())
            continue;

        if (checkCard(specs, cards[i])) {
            from->model().setGlow(i, true);
            eligibleCount++;
        }
    }
    if (eligibleCount) {
        mAbilityList->ability(mAbilityList->activeId()).effect = effect;
        if (effect.place->zone == asn::Zone::WaitingRoom ||
            effect.place->zone == asn::Zone::Deck)
            QMetaObject::invokeMethod(from->visualItem(), "openView", Q_ARG(QVariant, true));
        if (!event.mandatory())
            mAbilityList->activateCancel(mAbilityList->activeId(), true);
    } else {
        mGame->pause(800);
        sendGameCommand(CommandCancelEffect());
    }
}

void Player::processMoveChoice(const EventChooseMoveDestination &event) {
    if (mOpponent)
        return;

    auto effect = decodeMoveCard(event.effect());
    assert(effect.executor == asn::Player::Player);

    if (effect.to.size() == 1)
        return;

    std::vector<QString> data;
    for (const auto &to: effect.to) {
        assert(to.owner == asn::Owner::Player);
        assert(to.pos == asn::Position::NotSpecified);
        data.push_back(asnZoneToReadableString(to.zone));
    }

    mChoiceDialog->setData("Choose where to put the card", data);
}

void Player::activateAbilities(const EventAbilityActivated &event) {
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
            //if (protoa.type() == ProtoAbilityType::ProtoCard)
                //a.text =
        } else {
            a.code = protoa.cardcode();
            //if (protoa.type() == ProtoAbilityType::ProtoCard)
                //a.text =
        }
        if (protoa.type() == ProtoAbilityType::ProtoClimaxTrigger)
            a.text = printAbility(globalAbility(static_cast<TriggerIcon>(protoa.abilityid())));

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
        if (ef.placeType == asn::PlaceType::SpecificPlace &&
            ef.place->zone == asn::Zone::WaitingRoom) {
            QMetaObject::invokeMethod(zone("wr")->visualItem(), "openView", Q_ARG(QVariant, false));
        }
    }
}

void Player::abilityResolved() {
    mGame->pause(500);
    mAbilityList->removeActiveAbility();
}

void Player::makeAbilityActive(const EventPlayAbility &event) {
    mAbilityList->setActiveByUniqueId(event.uniqueid(), true);
}

void Player::cancelAbility(int) {
    doneChoosing();
    sendGameCommand(CommandCancelEffect());
}

void Player::chooseCard(int, QString qzone) {
    int activeId = mAbilityList->activeId();
    auto &effect = mAbilityList->ability(activeId).effect;
    if (!std::holds_alternative<asn::ChooseCard>(effect))
        return;

    auto &ef = std::get<asn::ChooseCard>(effect);
    assert(ef.targets.size() == 1);
    if (ef.targets[0].type != asn::TargetType::SpecificCards)
        return;

    auto pzone = zone(qzone.toStdString());
    int num = pzone->numOfSelectedCards();

    if (ef.targets[0].targetSpecification->number.value != num)
        return;

    mAbilityList->activateCancel(activeId, false);

    CommandChooseCard cmd;
    cmd.set_zone(qzone.toStdString());
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
