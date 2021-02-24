#include "print.h"

using namespace asn;

std::string printCard(const Card &c, bool plural, bool article) {
    std::string s;

    if (c.cardSpecifiers.empty()) {
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

    for (const auto &cardSpec: c.cardSpecifiers) {
        if (cardSpec.type == CardSpecifierType::Trait) {
            s += printTrait(std::get<Trait>(cardSpec.specifier).value) + " ";
        }
    }

    for (const auto &cardSpec: c.cardSpecifiers) {
        if (cardSpec.type == CardSpecifierType::CardType) {
            if (std::get<CardType>(cardSpec.specifier) == CardType::Char) {
                s += "character";
                if (plural)
                    s += 's';
                s += " ";
            }
        }
    }

    for (const auto &cardSpec: c.cardSpecifiers) {
        if (cardSpec.type == CardSpecifierType::TriggerIcon)
            if (std::get<TriggerIcon>(cardSpec.specifier) == TriggerIcon::Soul)
                s += "with a soul trigger ";
    }


    if (s[s.size() - 1] == ' ')
        s.pop_back();

    return s;
}
