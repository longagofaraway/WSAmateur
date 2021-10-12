#include "print.h"

#include <cassert>
#include <exception>


using namespace asn;

class PrintingException : public std::exception {};

std::string printDigit(int8_t value) {
    switch (value) {
    case 1: return "one";
    case 2: return "two";
    case 3: return "three";
    case 4: return "four";
    case 5: return "five";
    case 6: return "six";
    default: throw PrintingException();
    }
}

std::string printTrait(const std::string &trait) {
    return "&lt;" + trait + "&gt;";
}

std::string printZone(Zone zone) {
    switch (zone) {
    case Zone::NotSpecified: return "";
    case Zone::Clock: return "clock";
    case Zone::Deck: return "deck";
    case Zone::Hand: return "hand";
    case Zone::Level: return "level zone";
    case Zone::Memory: return "memory";
    case Zone::Stage: return "stage";
    case Zone::Stock: return "stock";
    case Zone::WaitingRoom: return "waiting room";
    case Zone::Climax: return "climax area";
    default: throw PrintingException();
    }
}

std::string printTriggerIcon(TriggerIcon icon) {
    switch (icon) {
    case TriggerIcon::Bag: return "bag";
    case TriggerIcon::Book: return "book";
    case TriggerIcon::Choice: return "choice";
    case TriggerIcon::Door: return "door";
    case TriggerIcon::Gate: return "gate";
    case TriggerIcon::Shot: return "shot";
    case TriggerIcon::Soul: return "soul";
    case TriggerIcon::Standby: return "standby";
    case TriggerIcon::Treasure: return "treasure";
    case TriggerIcon::Wind: return "wind";
    default: throw PrintingException();
    }
}

namespace {
bool cardHasAttr(const Card &c) {
    for (const auto &spec: c.cardSpecifiers) {
        if (spec.type == CardSpecifierType::Cost ||
            spec.type == CardSpecifierType::Level ||
            spec.type == CardSpecifierType::Power)
            return true;
    }
    return false;
}
}

std::string printTarget(const Target &t, bool plural, bool nominative, std::optional<bool> optArticle) {
    std::string s;

    if (t.type == TargetType::ThisCard)
        s += "this card ";
    else if (t.type == TargetType::ChosenCards) {
        if (gPrintState.chosenCardsNumber.value == 1)
            s += "it ";
        else
            s += nominative ? "they " : "them ";
    } else if (t.type == TargetType::SpecificCards) {
        const auto &spec = *t.targetSpecification;
        bool article = true;
        if (spec.mode == TargetMode::AllOther && spec.number.value != 1) {
            plural = true;
            article = false;
        }

        if (optArticle)
            article = optArticle.value();
        s += printCard(spec.cards, plural, article, spec.mode) + " ";

        // it's for multiplier
        if (spec.mode == TargetMode::FrontRowOther)
            s += "in the center stage ";
        else if (spec.mode == TargetMode::BackRowOther)
            s += "in the back stage ";
    } else if (t.type == TargetType::OppositeThis) {
        s += "the character facing this card ";
    } else if (t.type == TargetType::BattleOpponent) {
        if (gPrintState.battleOpponentMentioned) {
            gPrintState.battleOpponentMentioned = false;
            s += "that character ";
        } else {
            gPrintState.battleOpponentMentioned = true;
            s += "this card's battle opponent ";
        }
        if (t.targetSpecification) {
            const auto &spec = *t.targetSpecification;
            if (cardHasAttr(spec.cards)) {
                s += "with ";
                s += printCard(spec.cards, false, false, spec.mode) + " ";
            }
        }
    } else if (t.type == TargetType::MentionedInTrigger) {
        s += "that character ";
    }

    return s;
}

std::string printNumber(const Number &n, bool lowerHigher) {
    std::string s;

    if (lowerHigher) {
        s += std::to_string(n.value);
        if (n.mod == NumModifier::UpTo)
            s += " or lower ";
        else if (n.mod == NumModifier::AtLeast)
            s += " or higher ";
    } else {
        if (n.mod == NumModifier::UpTo)
            s += "up to ";
        else if (n.mod == NumModifier::AtLeast)
            s += "at least ";
        s += std::to_string(n.value) + " ";
    }
    return s;
}

std::string printState(State s) {
    switch (s) {
    case State::Reversed:
        return "<img align=\"middle\" src=\"qrc:///resources/images/reverse.png\">";
    case State::Standing:
        return "<img align=\"middle\" src=\"qrc:///resources/images/stand.png\">";
    case State::Rested:
        return "<img align=\"middle\" src=\"qrc:///resources/images/rest.png\">";
    }
    return "";
}

std::string printPlayer(Player p, Player executor) {
    std::string s;

    switch (p) {
    case Player::Player:
        s += "your ";
        break;
    case Player::Both:
        s += "your or your opponent's ";
        break;
    case Player::Opponent:
        if (executor == Player::Player)
            s += "your opponent's ";
        else
            s += "their ";
        break;
    case Player::NotSpecified:
    default:
        break;
    }

    return s;
}

std::string printKeyword(Keyword keyword) {
    switch (keyword) {
    case Keyword::Alarm:
        return "Alarm";
    case Keyword::Assist:
        return "Assist";
    case Keyword::Backup:
        return "Backup";
    case Keyword::Bond:
        return "Bond";
    case Keyword::Brainstorm:
        return "Brainstorm";
    case Keyword::Change:
        return "Change";
    case Keyword::Cxcombo:
        return "CXCombo";
    case Keyword::Encore:
        return "Encore";
    case Keyword::Experience:
        return "Experience";
    case Keyword::Replay:
        return "Replay";
    case Keyword::Resonance:
        return "Resonance";
    default:
        return "";
    }
}

std::string printKeywords(const std::vector<Keyword> &keywords) {
    std::string s;

    for (auto keyword: keywords)
        s += "<b>" + printKeyword(keyword) + "</b> ";

    return s;
}

std::string printPhase(Phase p) {
    switch (p) {
    case Phase::AttackPhase:
        return "attack phase";
    case Phase::ClimaxPhase:
        return "climax phase";
    case Phase::DrawPhase:
        return "draw phase";
    case Phase::EncoreStep:
        return "encore step";
    default:
        break;
    }
    return "";
}

bool haveExactName(const std::vector<CardSpecifier> &s) {
    for (const auto &c: s)
        if (c.type == CardSpecifierType::ExactName)
            return true;
    return false;
}

std::string printForEachMultiplier(const ForEachMultiplier &m, bool addOf) {
    std::string s;

    bool plural = true;
    std::optional<bool> article;
    if (m.target->type == TargetType::SpecificCards && !addOf) {
        const auto &spec = m.target->targetSpecification.value();
        bool hasOwnership = std::any_of(spec.cards.cardSpecifiers.begin(),
                                        spec.cards.cardSpecifiers.end(),
                                        [](const auto &cardSpec) {
            return cardSpec.type == CardSpecifierType::Owner;
        });
        if (hasOwnership)
            s += "of ";
        else {
            plural = false;
            article = false;
        }
    } else if (addOf) {
        s += "of ";
    }

    s += printTarget(*m.target, plural, false, article);

    if (m.placeType == PlaceType::SpecificPlace) {
        if (m.place->zone != Zone::Stage)
            s += "in your " + printZone(m.place->zone) + " ";
    } else if (m.placeType == PlaceType::LastMovedCards) {
        s += "among those cards ";
    }

    return s;
}

std::string printAttackType(AttackType t) {
    switch (t) {
    case AttackType::FronalAttack:
        return "frontal attack";
    case AttackType::SideAttack:
        return "side attack";
    case AttackType::Any:
        return "attack";
    }
    assert(false);
    return "";
}

std::string printDuration(int duration) {
    if (duration == 1)
        return "until end of turn ";
    if (duration == 2)
        return "until end of your opponent's turn ";
    return "";
}

std::string printPlace(Place place) {
    std::string s;
    if (place.pos == Position::FrontRow ||
        place.pos == Position::BackRow)
        s += "in " + printPlayer(place.owner);

    switch(place.pos) {
    case Position::FrontRow:
        return s + "center stage";
    case Position::BackRow:
        return s + "back stage";
    default:
        break;
    }

    if (place.pos == Position::Top) {
        s += "the top card of ";
        s += printPlayer(place.owner);
        s += printZone(place.zone);
    }
    return s;
}

