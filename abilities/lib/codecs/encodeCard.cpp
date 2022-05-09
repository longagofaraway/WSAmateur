#include "encode.h"

#include "encDecUtils.h"

using namespace asn;

void encodeLevelWithMultiplier(const LevelWithMultiplier &c, Buf &buf) {
    encodeNumber(c.value, buf);
    encodeMultiplier(c.multiplier, buf);
}

void encodeCardSpecifier(const CardSpecifier &c, Buf &buf) {
    buf.push_back(static_cast<uint8_t>(c.type));
    switch (c.type) {
    case CardSpecifierType::CardType:
        buf.push_back(static_cast<uint8_t>(std::get<CardType>(c.specifier)));
        break;
    case CardSpecifierType::Owner:
        buf.push_back(static_cast<uint8_t>(std::get<Player>(c.specifier)));
        break;
    case CardSpecifierType::Trait:
        encodeString(std::get<Trait>(c.specifier).value, buf);
        break;
    case CardSpecifierType::ExactName:
        encodeString(std::get<ExactName>(c.specifier).value, buf);
        break;
    case CardSpecifierType::NameContains:
        encodeString(std::get<NameContains>(c.specifier).value, buf);
        break;
    case CardSpecifierType::Level:
        encodeNumber(std::get<Level>(c.specifier).value, buf);
        break;
    case CardSpecifierType::Color:
        buf.push_back(static_cast<uint8_t>(std::get<Color>(c.specifier)));
        break;
    case CardSpecifierType::Cost:
        encodeNumber(std::get<CostSpecifier>(c.specifier).value, buf);
        break;
    case CardSpecifierType::TriggerIcon:
        buf.push_back(static_cast<uint8_t>(std::get<TriggerIcon>(c.specifier)));
        break;
    case CardSpecifierType::Power:
        encodeNumber(std::get<Power>(c.specifier).value, buf);
        break;
    case CardSpecifierType::LevelWithMultiplier:
        encodeLevelWithMultiplier(std::get<LevelWithMultiplier>(c.specifier), buf);
        break;
    default:
        break;
    }
}

void encodeCard(const Card &t, Buf &buf) {
    encodeArray(t.cardSpecifiers, buf, encodeCardSpecifier);
}
