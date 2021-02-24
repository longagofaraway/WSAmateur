#include "decode.h"

using namespace asn;

CardSpecifier decodeCardSpecifier(Iterator &it, Iterator end) {
    CardSpecifier c;

    c.type = decodeEnum<CardSpecifierType>(it, end);
    switch (c.type) {
    case CardSpecifierType::CardType:
        c.specifier = decodeEnum<CardType>(it, end);
        break;
    case CardSpecifierType::Owner:
        c.specifier = decodeEnum<Player>(it, end);
        break;
    case CardSpecifierType::Trait:
        c.specifier = Trait{ decodeString(it, end) };
        break;
    case CardSpecifierType::ExactName:
        c.specifier = ExactName{ decodeString(it, end) };
        break;
    case CardSpecifierType::NameContains:
        c.specifier = NameContains{ decodeString(it, end) };
        break;
    case CardSpecifierType::Level:
        c.specifier = Level{ decodeNumber(it, end) };
        break;
    case CardSpecifierType::Color:
        c.specifier = decodeEnum<Color>(it, end);
        break;
    case CardSpecifierType::Cost:
        c.specifier = CostSpecifier{ decodeNumber(it, end) };
        break;
    case CardSpecifierType::TriggerIcon:
        c.specifier = decodeEnum<TriggerIcon>(it, end);
        break;
    default:
        break;
    }

    return c;
}

Card decodeCard(Iterator &it, Iterator end) {
    Card c;
    c.cardSpecifiers = decodeArray<CardSpecifier>(it, end, decodeCardSpecifier);
    return c;
}
