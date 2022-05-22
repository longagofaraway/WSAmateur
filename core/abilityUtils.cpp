#include "abilityUtils.h"

#include <cassert>
#include <random>

#include <QtDebug>
#include <QByteArray>

#include "../client/src/card.h"
#include "abilityPlayer.h"
#include "serverCardZone.h"
#include "serverPlayer.h"

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

QString placeToReadableString(const asn::Place &place) {
    assert(place.owner == asn::Player::Player);
    auto result = asnZoneToReadableString(place.zone);
    if (place.pos == asn::Position::Top)
        result = "Top " + result;
    else if (place.pos == asn::Position::Bottom)
        result = "Bottom " + result;
    return result;
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

bool checkCard(const std::vector<asn::CardSpecifier> &specs, const CardBase &card, AbilityPlayer *abilityPlayer) {
    std::unordered_map<asn::CardSpecifierType, bool> metRequirements;
    for (const auto &spec: specs) {
        if (!metRequirements.contains(spec.type))
            metRequirements.emplace(spec.type, false);
        switch (spec.type) {
        case asn::CardSpecifierType::CardType:
            if (std::get<asn::CardType>(spec.specifier) == card.type())
                metRequirements[spec.type] = true;
            break;
        case asn::CardSpecifierType::TriggerIcon: {
            if (std::any_of(card.triggers().begin(), card.triggers().end(),
                [&spec](TriggerIcon icon) {
                    return icon == std::get<asn::TriggerIcon>(spec.specifier);
            }))
                metRequirements[spec.type] = true;
            break;
        }
        case asn::CardSpecifierType::Trait: {
            if (std::any_of(card.traits().begin(), card.traits().end(),
                [&spec](const std::string &trait) {
                    return trait == std::get<asn::Trait>(spec.specifier).value;
            }))
                metRequirements[spec.type] = true;
            break;
        }
        case asn::CardSpecifierType::Level: {
            const auto &number = std::get<asn::Level>(spec.specifier).value;
            if (checkNumber(number, card.level()))
                metRequirements[spec.type] = true;
            break;
        }
        case asn::CardSpecifierType::Power: {
            const auto &number = std::get<asn::Power>(spec.specifier).value;
            if (checkNumber(number, card.power()))
                metRequirements[spec.type] = true;
            break;
        }
        case asn::CardSpecifierType::Cost: {
            const auto &number = std::get<asn::CostSpecifier>(spec.specifier).value;
            if (checkNumber(number, card.cost()))
                metRequirements[spec.type] = true;
            break;
        }
        case asn::CardSpecifierType::ExactName: {
            const auto &name = std::get<asn::ExactName>(spec.specifier).value;
            if (name == card.name())
                metRequirements[spec.type] = true;
            break;
        }
        case asn::CardSpecifierType::Color: {
            const auto &neededColor = std::get<asn::Color>(spec.specifier);
            if (card.color() == neededColor)
                metRequirements[spec.type] = true;
            break;
        }
        case asn::CardSpecifierType::NameContains: {
            const auto &name = std::get<asn::NameContains>(spec.specifier).value;
            if (card.name().find(name) != std::string::npos)
                metRequirements[spec.type] = true;
            break;
        }
        case asn::CardSpecifierType::LevelHigherThanOpp:
            if (card.playersLevel() < card.level())
                metRequirements[spec.type] = true;
            break;
        case asn::CardSpecifierType::StandbyTarget:
            if (card.level() <= card.playersLevel() + 1)
                metRequirements[spec.type] = true;
            break;
        case asn::CardSpecifierType::LevelWithMultiplier: {
            const auto &level = std::get<asn::LevelWithMultiplier>(spec.specifier);
            asn::Number number;
            number.mod = level.value.mod;
            if (!abilityPlayer) {
                qWarning() << "checking LevelWithMultiplier on client side, shoudn't happen!";
                break;
            }
            if (level.multiplier.type == asn::MultiplierType::AddLevel)
                number.value = abilityPlayer->getAddLevelMultiplierValue(level.multiplier) + level.value.value;
            else
                throw std::runtime_error("unhandled multiplier");
            if (checkNumber(number, card.level()))
                metRequirements[spec.type] = true;
            break;
        }
        case asn::CardSpecifierType::State:
            if (std::get<asn::State>(spec.specifier) == card.state())
                metRequirements[spec.type] = true;
            break;
        case asn::CardSpecifierType::Owner:
            // don't process here
            break;
        default:
            assert(false);
            break;
        }
    }
    for (const auto &pair: metRequirements) {
        if (!pair.second)
            return false;
    }
    return true;
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
    case asn::TargetMode::BackRowOther:
        return isBackRow(card->pos()) && thisCard != card;
    case asn::TargetMode::FrontRowOther:
        return isFrontRow(card->pos()) && thisCard != card;
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

asn::AttackType protoAttackTypeToAttackType(AttackType attackType) {
    switch (attackType) {
    case AttackType::SideAttack:
        return asn::AttackType::Side;
    case AttackType::FrontAttack:
        return asn::AttackType::Frontal;
    case AttackType::DirectAttack:
        return asn::AttackType::Direct;
    }
    assert(false);
    return asn::AttackType::Frontal;
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

ProtoCardBoolAttribute getProtoBoolAttrType(BoolAttributeType type) {
    switch (type) {
    case BoolAttributeType::CannotFrontAttack:
        return ProtoCannotFrontAttack;
    case BoolAttributeType::CannotSideAttack:
        return ProtoCannotSideAttack;
    case BoolAttributeType::CannotBecomeReversed:
        return ProtoCannotBecomeReversed;
    case BoolAttributeType::CannotMove:
        return ProtoCannotMove;
    case BoolAttributeType::SideAttackWithoutPenalty:
        return ProtoSideAttackWithoutPenalty;
    case BoolAttributeType::CannotStand:
        return ProtoCannotStand;
    case BoolAttributeType::CannotBeChosen:
        return ProtoCannotBeChosen;
    case BoolAttributeType::CanPlayWithoutColorRequirement:
        return ProtoPlayWithoutColorRequirement;
    }
    assert(false);
    return ProtoCannotFrontAttack;
}

ProtoPlayerAttribute getProtoPlayerAttrType(PlayerAttrType type) {
    switch (type) {
    case PlayerAttrType::CannotPlayBackups:
        return ProtoCannotPlayBackups;
    case PlayerAttrType::CannotPlayEvents:
        return ProtoCannotPlayEvents;
    case PlayerAttrType::CharAutoCannotDealDamage:
        return ProtoCharAutoCannotDealDamage;
    }
    assert(false);
    return ProtoCharAutoCannotDealDamage;
}

asn::Player reversePlayer(asn::Player p) {
    switch(p) {
    case asn::Player::Player:
        return asn::Player::Opponent;
    case asn::Player::Opponent:
        return asn::Player::Player;
    }
    return p;
}

bool isPositional(const asn::Target &t) {
    if (t.type == asn::TargetType::SpecificCards) {
        const auto &spec = *t.targetSpecification;
        if (spec.mode == asn::TargetMode::InFrontOfThis || spec.mode == asn::TargetMode::BackRow
            || spec.mode == asn::TargetMode::FrontRow)
            return true;
    } else if (t.type == asn::TargetType::OppositeThis) {
        return true;
    }
    return false;
}

bool checkPlace(const ServerCard *card, const asn::Place &place) {
    if (card->zone()->name() != asnZoneToString(place.zone))
        return false;
    switch (place.pos) {
    case asn::Position::FrontRow:
        return isFrontRow(card->pos());
    case asn::Position::BackRow:
        return isBackRow(card->pos());
    case asn::Position::Top:
        return card->pos() == (card->zone()->count() - 1);
    case asn::Position::OppositeCharacter:
        return card->player()->oppositeCard(card) != nullptr;
    }
    assert(false);
    return true;
}
