#include "serverPlayer.h"

#include "abilityPlayer.h"
#include "abilityUtils.h"
#include "serverGame.h"


bool AbilityPlayer::evaluateCondition(const asn::Condition &c) {
    switch (c.type) {
    case asn::ConditionType::NoCondition:
        return true;
    case asn::ConditionType::IsCard:
        return evaluateConditionIsCard(std::get<asn::ConditionIsCard>(c.cond));
    case asn::ConditionType::HaveCards:
        return evaluateConditionHaveCard(std::get<asn::ConditionHaveCard>(c.cond));
    case asn::ConditionType::And:
        return evaluateConditionAnd(std::get<asn::ConditionAnd>(c.cond));
    case asn::ConditionType::InBattleWithThis:
        return evaluateConditionInBattleWithThis();
    case asn::ConditionType::SumOfLevels:
        return evaluateConditionSumOfLevels(std::get<asn::ConditionSumOfLevels>(c.cond));
    case asn::ConditionType::DuringTurn:
        return evaluateConditionDuringTurn(std::get<asn::ConditionDuringTurn>(c.cond));
    case asn::ConditionType::CheckOpenedCards:
        return evaluateConditionCheckOpenedCards(std::get<asn::ConditionCheckOpenedCards>(c.cond));
    default:
        assert(false);
        return false;
    }
}

bool AbilityPlayer::evaluateConditionIsCard(const asn::ConditionIsCard &c) {
    if (c.target.type == asn::TargetType::MentionedCards ||
        c.target.type == asn::TargetType::LastMovedCards) {
        const auto &cards = c.target.type == asn::TargetType::MentionedCards ?
                    mentionedCards() : lastMovedCards();
        if (cards.empty())
            return false;
        for (const auto &card: cards) {
            assert(card.card);
            bool ok = false;
            for (const auto &neededCard: c.neededCard) {
                if (checkCard(neededCard.cardSpecifiers, *card.card)) {
                    ok = true;
                    break;
                }
            }
            if (!ok)
                return false;
        }
        return true;
    } else if (c.target.type == asn::TargetType::SpecificCards) {
        assert(c.neededCard.size() == 1);
        const auto &spec = *c.target.targetSpecification;
        if (spec.mode == asn::TargetMode::All) {
            auto stage = mPlayer->zone("stage");
            bool verified = true;
            for (int i = 0; i < stage->count(); ++i)
                if (stage->card(i) && !checkCard(c.neededCard[0].cardSpecifiers, *stage->card(i)))
                    verified = false;
            if (verified)
                return true;
        }
    } else if (c.target.type == asn::TargetType::BattleOpponent) {
        if (thisCard().card->zone()->name() != "stage")
            return false;
        auto card = mPlayer->oppositeCard(thisCard().card);
        if (!card)
            return false;

        return checkCard(c.neededCard[0].cardSpecifiers, *card);
    }
    return false;
}

bool AbilityPlayer::evaluateConditionHaveCard(const asn::ConditionHaveCard &c) {
    assert(c.who != asn::Player::Both);
    auto z = owner(c.who)->zone(asnZoneToString(c.where.zone));
    int count = 0;
    for (int i = 0; i < z->count(); ++i) {
        auto card = z->card(i);
        if (!card)
            continue;

        if (c.excludingThis && thisCard().card == card)
            continue;

        if (checkCard(c.whichCards.cardSpecifiers, *card)) {
            count++;

            if (c.howMany.mod == asn::NumModifier::AtLeast &&
                count >= c.howMany.value)
                return c.invert ? false : true;
        }
    }
    if ((c.howMany.mod == asn::NumModifier::ExactMatch &&
         c.howMany.value == count) ||
        (c.howMany.mod == asn::NumModifier::UpTo &&
         c.howMany.value >= count))
        return c.invert ? false : true;

    return c.invert ? true : false;
}

bool AbilityPlayer::evaluateConditionAnd(const asn::ConditionAnd &c) {
    bool res = true;
    for (const auto &cond: c.cond)
        res = res && evaluateCondition(cond);
    return res;
}

bool AbilityPlayer::evaluateConditionInBattleWithThis() {
    return thisCard().card->inBattle();
}

bool AbilityPlayer::evaluateConditionSumOfLevels(const asn::ConditionSumOfLevels &c) {
    auto level = mPlayer->zone("level");
    int sum = 0;
    for (int i = 0; i < level->count(); ++i)
        sum += level->card(i)->level();
    return sum > c.moreThan;
}

bool AbilityPlayer::evaluateConditionDuringTurn(const asn::ConditionDuringTurn &c) {
    if (c.player == asn::Player::Player)
        return mPlayer->active();
    else if (c.player == asn::Player::Opponent)
        return !mPlayer->active();
    else {
        assert(false);
        return true;
    }
}

bool AbilityPlayer::evaluateConditionCheckOpenedCards(const asn::ConditionCheckOpenedCards &c) {
    auto &thoseCards = lastMovedCards();
    int count = 0;
    for (int i = 0; i < thoseCards.size(); ++i) {
        if (checkCard(c.card.cardSpecifiers, *thoseCards[i].card) &&
            checkNumber(c.number, ++count))
            return true;
    }
    return false;
}
