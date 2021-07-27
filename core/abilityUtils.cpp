#include "abilityUtils.h"

#include <cassert>
#include <random>

#include <QByteArray>

#include "../client/src/card.h"

std::string_view asnZoneToString(asn::Zone zone) {
    switch (zone) {
    case asn::Zone::Climax:
        return "climax";
    case asn::Zone::Clock:
        return "clock";
    case asn::Zone::Deck:
        return "deck";
    case asn::Zone::Hand:
        return "hand";
    case asn::Zone::Level:
        return "level";
    case asn::Zone::Memory:
        return "memory";
    case asn::Zone::Stage:
        return "stage";
    case asn::Zone::Stock:
        return "stock";
    case asn::Zone::WaitingRoom:
        return "wr";
    default:
        assert(false);
        return "";
    }
}

QString asnZoneToReadableString(asn::Zone zone) {
    switch (zone) {
    case asn::Zone::Climax:
        return "Climax";
    case asn::Zone::Clock:
        return "Clock";
    case asn::Zone::Deck:
        return "Deck";
    case asn::Zone::Hand:
        return "Hand";
    case asn::Zone::Level:
        return "Level";
    case asn::Zone::Memory:
        return "Memory";
    case asn::Zone::Stage:
        return "Stage";
    case asn::Zone::Stock:
        return "Stock";
    case asn::Zone::WaitingRoom:
        return "Waiting Room";
    default:
        assert(false);
        return "";
    }
}

uint32_t abilityHash(const ProtoAbility &a) {
    std::string buf = std::to_string(a.card_id());
    buf += std::to_string(a.ability_id());
    buf += std::to_string(a.type());

    std::mt19937 gen(static_cast<unsigned>(time(nullptr)));
    buf += std::to_string(gen() % 0xFFFF);
    auto checksum = qChecksum(buf.data(), static_cast<uint>(buf.size()));
    if (!checksum)
        checksum++;
    return checksum;
}

ProtoCardAttribute attrTypeToProto(asn::AttributeType t) {
    switch (t) {
    case asn::AttributeType::Soul:
        return ProtoAttrSoul;
    case asn::AttributeType::Power:
        return ProtoAttrPower;
    case asn::AttributeType::Level:
        return ProtoAttrLevel;
    default:
        assert(false);
        return ProtoAttrSoul;
    }
}

bool checkCard(const std::vector<asn::CardSpecifier> &specs, const CardBase &card) {
    bool eligible = true;
    bool hasNameContains = false;
    bool nameContains = false;
    bool hasExactName = false;
    bool exactNameMatch = false;
    for (const auto &spec: specs) {
        switch (spec.type) {
        case asn::CardSpecifierType::CardType:
            if (std::get<asn::CardType>(spec.specifier) != card.type())
                eligible = false;
            break;
        case asn::CardSpecifierType::TriggerIcon: {
            bool found = false;
            for (auto triggerIcon: card.triggers()) {
                if (triggerIcon == std::get<asn::TriggerIcon>(spec.specifier)) {
                    found = true;
                    break;
                }
            }
            if (!found)
                eligible = false;
            break;
        }
        case asn::CardSpecifierType::Trait: {
            bool found = false;
            for (auto trait: card.traits()) {
                if (trait == std::get<asn::Trait>(spec.specifier).value) {
                    found = true;
                    break;
                }
            }
            if (!found)
                eligible = false;
            break;
        }
        case asn::CardSpecifierType::Level: {
            const auto &number = std::get<asn::Level>(spec.specifier).value;
            if (!checkNumber(number, card.level()))
                eligible = false;
            break;
        }
        case asn::CardSpecifierType::Cost: {
            const auto &number = std::get<asn::CostSpecifier>(spec.specifier).value;
            if (!checkNumber(number, card.cost()))
                eligible = false;
            break;
        }
        case asn::CardSpecifierType::ExactName: {
            hasExactName = true;
            const auto &name = std::get<asn::ExactName>(spec.specifier).value;
            if (name == card.name())
                exactNameMatch = true;
            break;
        }
        case asn::CardSpecifierType::NameContains: {
            hasNameContains = true;
            const auto &name = std::get<asn::NameContains>(spec.specifier).value;
            if (card.name().find(name) != std::string::npos)
                nameContains = true;
            break;
        }
        case asn::CardSpecifierType::LevelHigherThanOpp:
            if (!card.levelGtPlayerLevel())
                eligible = false;
            break;
        case asn::CardSpecifierType::Owner:
            // don't process here
            break;
        default:
            assert(false);
            break;
        }
    }
    if ((hasNameContains && !nameContains) ||
        (hasExactName && !exactNameMatch))
        eligible = false;

    return eligible;
}

namespace {
bool isInFrontOf(int backPos, int frontPos) {
    if ((backPos == 3 && (frontPos == 0 || frontPos == 1))
        || (backPos == 4 && (frontPos == 1 || frontPos == 2)))
        return true;
    return false;
}
bool isBackRow(int pos) {
    return pos == 3 || pos == 4;
}
bool isFrontRow(int pos) {
    return pos >= 0 && pos <= 2;
}
}

bool checkTargetMode(asn::TargetMode mode, const ServerCard *thisCard, const ServerCard *card) {
    switch (mode) {
    case asn::TargetMode::AllOther:
        return thisCard != card;
    case asn::TargetMode::InFrontOfThis:
        return isInFrontOf(thisCard->pos(), card->pos());
    case asn::TargetMode::BackRow:
        return isBackRow(card->pos());
    case asn::TargetMode::FrontRow:
        return isFrontRow(card->pos());
    }

    return true;
}

bool checkNumber(const asn::Number &numObj, int n) {
    if ((numObj.mod == asn::NumModifier::ExactMatch &&
         numObj.value == n) ||
        (numObj.mod == asn::NumModifier::AtLeast &&
         numObj.value <= n) ||
        (numObj.mod == asn::NumModifier::UpTo &&
         numObj.value >= n))
        return true;
    return false;
}

asn::Player protoPlayerToPlayer(ProtoOwner player) {
    switch (player) {
    case ProtoPlayer:
        return asn::Player::Player;
    case ProtoOpponent:
        return asn::Player::Opponent;
    }
    assert(false);
    return asn::Player::Player;
}

asn::State protoStateToState(CardState state) {
    switch (state) {
    case StateStanding:
        return asn::State::Standing;
    case StateRested:
        return asn::State::Rested;
    case StateReversed:
        return asn::State::Reversed;
    }
    assert(false);
    return asn::State::Standing;
}

CardState stateToProtoState(asn::State state) {
    switch (state) {
    case asn::State::Standing:
        return StateStanding;
    case asn::State::Rested:
        return StateRested;
    case asn::State::Reversed:
        return StateReversed;
    }
    assert(false);
    return StateStanding;
}