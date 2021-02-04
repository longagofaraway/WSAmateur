#include "print.h"

using namespace asn;

std::string printCard(const Card &c, bool plural) {
    std::string s;

    if (!plural)
        s += "a ";

    for (const auto &cardSpec: c.cardSpecifiers) {
        if (cardSpec.type == CardSpecifierType::Owner) {
            if (std::get<Owner>(cardSpec.specifier) == Owner::Player)
                s += "your ";
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
