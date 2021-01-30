#include "print.h"

using namespace asn;

std::string printCard(const Card &c, bool plural) {
    std::string s;

    if (c.cardSpecifiers.size() == 1 && c.cardSpecifiers[0].type == CardSpecifierType::CardType
            && !plural)
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
            }
        }
    }

    return s;
}
