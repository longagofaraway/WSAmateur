#include "serverPlayer.h"

#include "abilityUtils.h"
#include "serverGame.h"


bool ServerPlayer::evaluateCondition(const asn::Condition &c) {
    switch (c.type) {
    case asn::ConditionType::NoCondition:
        return true;
    case asn::ConditionType::IsCard:
        return evaluateConditionIsCard(std::get<asn::ConditionIsCard>(c.cond));
    case asn::ConditionType::HaveCards:
        return evaluateConditionHaveCard(std::get<asn::ConditionHaveCard>(c.cond));
    case asn::ConditionType::And:
        return evaluateConditionAnd(std::get<asn::ConditionAnd>(c.cond));
    default:
        assert(false);
        return false;
    }
}

bool ServerPlayer::evaluateConditionIsCard(const asn::ConditionIsCard &c) {
    if (c.target.type == asn::TargetType::MentionedCards) {
        for (const auto &card: mContext.mentionedCards) {
            assert(card.card);
            for (const auto &neededCard: c.neededCard)
                if (checkCard(neededCard.cardSpecifiers, *card.card))
                    return true;
        }
    } else if (c.target.type == asn::TargetType::SpecificCards) {
        assert(c.neededCard.size() == 1);
        const auto &spec = *c.target.targetSpecification;
        if (spec.mode == asn::TargetMode::All) {
            auto stage = zone("stage");
            bool verified = true;
            for (int i = 0; i < stage->count(); ++i)
                if (stage->card(i) && !checkCard(c.neededCard[0].cardSpecifiers, *stage->card(i)))
                        verified = false;
            if (verified)
                return true;
        }
    }
    return false;
}

bool ServerPlayer::evaluateConditionHaveCard(const asn::ConditionHaveCard &c) {
    assert(c.invert == false);
    assert(c.who != asn::Player::Both);
    auto player = (c.who == asn::Player::Player) ? this : mGame->opponentOfPlayer(mId);
    auto z = player->zone(asnZoneToString(c.where.zone));
    int count = 0;
    for (int i = 0; i < z->count(); ++i) {
        auto card = z->card(i);
        if (!card)
            continue;

        if (c.excludingThis && mContext.thisCard.card == card)
            continue;

        if (checkCard(c.whichCards.cardSpecifiers, *card)) {
            count++;

            if (c.howMany.mod == asn::NumModifier::AtLeast &&
                count >= c.howMany.value)
                return true;
        }
    }
    if ((c.howMany.mod == asn::NumModifier::ExactMatch &&
         c.howMany.value == count) ||
        (c.howMany.mod == asn::NumModifier::UpTo &&
         c.howMany.value >= count))
        return true;

    return false;
}

bool ServerPlayer::evaluateConditionAnd(const asn::ConditionAnd &c) {
    bool res = true;
    for (const auto &cond: c.cond)
        res = res && evaluateCondition(cond);
    return res;
}
