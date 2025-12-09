#include "abilityPlayer.h"

#include "abilityEvents.pb.h"
#include "abilityCommands.pb.h"

#include "serverPlayer.h"

Resumable AbilityPlayer::playS79_20() {
    auto opponent = mPlayer->getOpponent();
    auto pDeck = mPlayer->zone("deck");
    auto oDeck = opponent->zone("deck");

    EventRevealTopDeck eventPlayer;
    eventPlayer.set_card_id(pDeck->card(pDeck->count() - 1)->id());
    eventPlayer.set_code(pDeck->card(pDeck->count() - 1)->code());
    mPlayer->sendToBoth(eventPlayer);

    EventRevealTopDeck eventOpponent;
    eventOpponent.set_card_id(oDeck->card(oDeck->count() - 1)->id());
    eventOpponent.set_code(oDeck->card(oDeck->count() - 1)->code());
    opponent->sendToBoth(eventOpponent);

    auto pCard = pDeck->topCard();
    auto oCard = oDeck->topCard();

    if (pCard->level() > oCard->level()) {
        asn::ChooseCard c;
        asn::TargetAndPlace tp;
        asn::Target t;
        t.type = asn::TargetType::SpecificCards;
        asn::TargetSpecificCards spec;
        spec.mode = asn::TargetMode::Any;
        spec.number = asn::Number{asn::NumModifier::ExactMatch, 1};
        spec.cards.cardSpecifiers.push_back(asn::CardSpecifier{asn::CardSpecifierType::CardType, asn::CardType::Char});
        spec.cards.cardSpecifiers.push_back(asn::CardSpecifier{asn::CardSpecifierType::Trait, asn::Trait{"Shuchiin"}});
        t.targetSpecification = spec;
        tp.target = t;
        tp.placeType = asn::PlaceType::SpecificPlace;
        tp.place = asn::Place{asn::Position::NotSpecified, asn::Zone::WaitingRoom, asn::Player::Player};
        c.targets.push_back(tp);
        c.executor = asn::Player::Player;

        co_await playChooseCard(c);

        asn::MoveCard m;
        asn::Target t2;
        t2.type = asn::TargetType::ChosenCards;
        m.target = t2;
        m.executor = asn::Player::Player;
        m.order = asn::Order::NotSpecified;
        m.from = asn::Place{asn::Position::NotSpecified, asn::Zone::WaitingRoom, asn::Player::Player};
        m.to.push_back(asn::Place{asn::Position::NotSpecified, asn::Zone::Hand, asn::Player::Player});

        co_await playMoveCard(m);
    } else if (pCard->level() < oCard->level()) {
        asn::ChooseCard c;
        asn::TargetAndPlace tp;
        asn::Target t;
        t.type = asn::TargetType::SpecificCards;
        asn::TargetSpecificCards spec;
        spec.mode = asn::TargetMode::Any;
        spec.number = asn::Number{asn::NumModifier::ExactMatch, 1};
        spec.cards.cardSpecifiers.push_back(asn::CardSpecifier{asn::CardSpecifierType::CardType, asn::CardType::Char});
        t.targetSpecification = spec;
        tp.target = t;
        tp.placeType = asn::PlaceType::SpecificPlace;
        tp.place = asn::Place{asn::Position::NotSpecified, asn::Zone::Stage, asn::Player::Opponent};
        c.targets.push_back(tp);
        c.executor = asn::Player::Opponent;

        co_await playChooseCard(c);

        asn::AttributeGain a;
        asn::Target t2;
        t2.type = asn::TargetType::ChosenCards;
        a.target = t2;
        a.type = asn::AttributeType::Power;
        a.gainType = asn::ValueType::RawValue;
        a.value = 5000;
        a.duration = 1;

        playAttributeGain(a);
    }
}
