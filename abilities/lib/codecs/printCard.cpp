#include "print.h"

using namespace asn;

std::string printCard(const Card &c, bool plural, bool article, TargetMode mode) {
    std::string s;

    if (c.cardSpecifiers.empty()) {
        if (!plural && article)
            s += "a ";
        s += "card";
        if (plural)
            s += "s";
        return s;
    }

    for (const auto &cardSpec: c.cardSpecifiers) {
        if (cardSpec.type == CardSpecifierType::ExactName) {
            s += "\"" + std::get<ExactName>(cardSpec.specifier).value + "\" ";
            article = false;
        }
    }

    if (!plural && article)
        s += "a ";

    for (const auto &cardSpec: c.cardSpecifiers) {
        if (cardSpec.type == CardSpecifierType::Owner) {
            switch (std::get<Player>(cardSpec.specifier)) {
            case Player::Player:
                s += "your ";
                break;
            case Player::Opponent:
                s += "your opponent's ";
                break;
            default:
                break;
            }
        }
    }

    for (const auto &cardSpec: c.cardSpecifiers) {
        if (cardSpec.type == CardSpecifierType::Level) {
            const auto &level = std::get<Level>(cardSpec.specifier);
            s += "level " + std::to_string(level.value.value) + " ";
            if (level.value.mod == NumModifier::UpTo)
                s += "or lower ";
            else if (level.value.mod == NumModifier::AtLeast)
                s += "or higher ";
        }
    }

    for (const auto &cardSpec: c.cardSpecifiers) {
        if (cardSpec.type == CardSpecifierType::Cost) {
            const auto &cost = std::get<CostSpecifier>(cardSpec.specifier);
            s += "cost " + std::to_string(cost.value.value) + " ";
            if (cost.value.mod == NumModifier::UpTo)
                s += "or lower ";
            else if (cost.value.mod == NumModifier::AtLeast)
                s += "or higher ";
        }
    }

    if (mode == TargetMode::AllOther)
        s += "other ";

    for (const auto &cardSpec: c.cardSpecifiers) {
        if (cardSpec.type == CardSpecifierType::Trait) {
            s += printTrait(std::get<Trait>(cardSpec.specifier).value) + " ";
        }
    }

    for (const auto &cardSpec: c.cardSpecifiers) {
        if (cardSpec.type == CardSpecifierType::CardType) {
            switch (std::get<CardType>(cardSpec.specifier)) {
            case CardType::Char:
                s += "character";
                break;
            case CardType::Climax:
                s += "climax";
                if (plural)
                    s += "e";
                break;
            case CardType::Event:
                if (s.substr(s.size() - 2, 2) == "a ") {
                    s[s.size() - 1] = 'n';
                    s.push_back(' ');
                }
                s += "event";
                break;
            case CardType::Marker:
                s += "marker";
                break;
            }
            if (plural)
                s += 's';
            s += " ";
        }
    }

    for (const auto &cardSpec: c.cardSpecifiers) {
        if (cardSpec.type == CardSpecifierType::TriggerIcon)
            if (std::get<TriggerIcon>(cardSpec.specifier) == TriggerIcon::Soul)
                s += "with a soul trigger ";
    }

    for (const auto &cardSpec: c.cardSpecifiers) {
        if (cardSpec.type == CardSpecifierType::NameContains)
            s += "with \"" + std::get<NameContains>(cardSpec.specifier).value + "\" in its card name ";
    }

    for (const auto &cardSpec: c.cardSpecifiers) {
        if (cardSpec.type == CardSpecifierType::LevelHigherThanOpp)
            s += "with level higher than your opponent's level ";
    }

    if (s[s.size() - 1] == ' ')
        s.pop_back();

    return s;
}
