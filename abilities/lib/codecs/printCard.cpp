#include "print.h"

std::string printCard(const AsnCard &c, bool plural) {
    std::string s;

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
            }
        }
    }

    return s;
}
