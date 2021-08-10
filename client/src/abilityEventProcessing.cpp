#include "player.h"

#include "abilityEvents.pb.h"
#include "abilityCommands.pb.h"

#include "abilityUtils.h"
#include "cardZone.h"
#include "game.h"
#include "hand.h"
#include "stage.h"
#include "codecs/decode.h"
#include "codecs/print.h"
#include "utils.h"

namespace {
template<typename F>
auto decodingWrapper(const std::string &buf, F &decodeFunction) {
    std::vector<uint8_t> binbuf(buf.begin(), buf.end());
    auto it = binbuf.cbegin();
    return decodeFunction(it, binbuf.end());
}
}

void Player::processChooseCard(const EventChooseCard &event) {
    auto effect = decodingWrapper(event.effect(), decodeChooseCard);
    if ((mOpponent && effect.executor == asn::Player::Player) ||
        (!mOpponent && effect.executor == asn::Player::Opponent))
        return;
    if (effect.targets.size() != 1 ||
        effect.targets[0].target.type != asn::TargetType::SpecificCards) {
        assert(false);
        return;
    }

    const auto &spec = *effect.targets[0].target.targetSpecification;
    bool mandatory = event.mandatory();
    if (spec.number.mod == asn::NumModifier::UpTo)
        mandatory = false;

    int eligibleCardsNum;
    bool fromView = effect.targets[0].placeType == asn::PlaceType::Selection;
    if (fromView)
        eligibleCardsNum = highlightEligibleCards(zone("view"), spec.cards.cardSpecifiers,
                                                  spec.mode, mAbilityList->ability(mAbilityList->activeId()));
    else
        eligibleCardsNum = highlightCardsForChoice(effect.targets[0].target, *effect.targets[0].place);
    if (eligibleCardsNum)
        mAbilityList->ability(mAbilityList->activeId()).effect = effect;
    processChooseCardInternal(eligibleCardsNum, fromView ? std::nullopt : OptionalPlace(*effect.targets[0].place),
                              mandatory, effect.executor);
}

void Player::processSearchCard(const EventSearchCard &event) {
    auto effect = decodingWrapper(event.effect(), decodeSearchCard);
    assert(effect.place.zone == asn::Zone::Deck);
    assert(effect.targets.size() == 1);
    assert(effect.targets[0].cards.size() == 1);
    mDeckView->setCards(event.codes());
    const auto &specs = effect.targets[0].cards[0].cardSpecifiers;
    int eligibleCount = highlightEligibleCards(mDeckView, specs, asn::TargetMode::Any,
                                               mAbilityList->ability(mAbilityList->activeId()));

    if (eligibleCount) {
        mAbilityList->ability(mAbilityList->activeId()).effect = effect;
        if (effect.targets[0].number.mod == asn::NumModifier::UpTo)
            mAbilityList->activateCancel(mAbilityList->activeId(), true);
    } else {
        mGame->pause(800);
        sendGameCommand(CommandCancelEffect());
    }
}

void Player::processMoveChoice(const EventMoveChoice &event) {
    if (mOpponent)
        return;

    auto effect = decodingWrapper(event.effect(), decodeMoveCard);
    assert(effect.executor == asn::Player::Player);
    assert(!event.mandatory());

    auto effectText = printMoveCard(effect) + '?';
    effectText[0] = std::toupper(effectText[0]);
    std::vector<QString> data { "Yes", "No" };
    auto choiceDlg = std::make_unique<ChoiceDialog>(mGame);
    choiceDlg->setData(QString::fromStdString(effectText), data);
    mChoiceDialog = std::move(choiceDlg);
}

void Player::processMoveDestinationChoice(const EventMoveDestinationChoice &event) {
    if (mOpponent)
        return;

    auto effect = decodingWrapper(event.effect(), decodeMoveCard);
    assert(effect.executor == asn::Player::Player);

    if (effect.to.size() > 1) {
        std::vector<QString> data;
        for (const auto &to: effect.to) {
            assert(to.owner == asn::Player::Player);
            assert(to.pos == asn::Position::NotSpecified);
            data.push_back(asnZoneToReadableString(to.zone));
        }

        auto choiceDlg = std::make_unique<ChoiceDialog>(mGame);
        choiceDlg->setData("Choose where to put the card", data);
        mChoiceDialog = std::move(choiceDlg);
        return;
    }
}

void Player::processMoveDestinationIndexChoice(const EventMoveDestinationIndexChoice &event) {
    auto effect = decodingWrapper(event.effect(), decodeMoveCard);
    if ((mOpponent && effect.executor == asn::Player::Player) ||
        (!mOpponent && effect.executor == asn::Player::Opponent))
        return;

    assert(effect.to.size() == 1);
    auto &a = mAbilityList->ability(mAbilityList->activeId());
    a.effect = effect;
    a.choiceType = ChoiceType::DestinationIndex;
    auto player = effect.to[0].owner == asn::Player::Opponent ? getOpponent() : this;
    auto stage = player->zone("stage");
    if (effect.to[0].pos == asn::Position::EmptySlotFrontRow) {
        for (int i = 0; i < 3; ++i) {
            if (!stage->cards()[i].cardPresent())
                stage->model().setGlow(i, true);
        }
    } if (effect.to[0].pos == asn::Position::EmptySlotBackRow) {
        // highlight all row, because otherwise the move will be either auto-performed or auto-declined
        stage->model().setGlow(3, true);
        stage->model().setGlow(4, true);
    } else if (effect.to[0].pos == asn::Position::EmptySlot) {
        for (int i = 0; i < stage->model().count(); ++i) {
            if (!stage->cards()[i].cardPresent())
                stage->model().setGlow(i, true);
        }
    } else if (effect.to[0].zone == asn::Zone::Stage && effect.to[0].pos == asn::Position::NotSpecified) {
        highlightAllCards(stage, true);
    }
}

void Player::processMoveTargetChoice(const EventMoveTargetChoice &event) {
    if (mOpponent)
        return;

    auto effect = decodingWrapper(event.effect(), decodeMoveCard);
    assert(effect.executor == asn::Player::Player);
    assert(effect.target.type == asn::TargetType::SpecificCards);
    assert(effect.from.pos == asn::Position::NotSpecified);

    auto effectText = printMoveCard(effect);
    effectText[0] = std::toupper(effectText[0]);
    mGame->showText(QString::fromStdString(effectText));
    int eligibleCount = highlightCardsForChoice(effect.target, effect.from);
    if (eligibleCount)
        mAbilityList->ability(mAbilityList->activeId()).effect = effect;
    processChooseCardInternal(eligibleCount, effect.from, event.mandatory(), effect.executor);
}

void Player::processDrawChoice(const EventDrawChoice &event) {
    if (mOpponent)
        return;

    auto effect = decodingWrapper(event.effect(), decodeDrawCard);

    auto header = printDrawCard(effect);
    header[0] = std::toupper(header[0]);
    header.push_back('?');
    if (!event.mandatory()) {
        std::vector<QString> data { "Yes", "No" };

        auto choiceDlg = std::make_unique<ChoiceDialog>(mGame);
        choiceDlg->setData(QString::fromStdString(header), data);
        mChoiceDialog = std::move(choiceDlg);
    }
}

void Player::processAbilityChoice(const EventAbilityChoice &event) {
    if (mOpponent)
        return;

    auto effect = decodingWrapper(event.effect(), decodeAbilityGain);
    if (static_cast<size_t>(effect.number) < effect.abilities.size()) {
        std::vector<QString> data;
        for (const auto &a: effect.abilities)
            data.push_back(QString::fromStdString(printAbility(a)));

        auto choiceDlg = std::make_unique<ChoiceDialog>(mGame);
        choiceDlg->setData("Choose ability", data);
        mChoiceDialog = std::move(choiceDlg);
        mAbilityList->ability(mAbilityList->activeId()).effect = effect;
    }
}

void Player::processEffectChoice(const EventEffectChoice &event) {
    if (mOpponent)
        return;

    auto effect = decodingWrapper(event.effect(), decodePerformEffect);
    if (static_cast<size_t>(effect.numberOfEffects) >= effect.effects.size())
        return;

    std::vector<QString> data;
    for (const auto &a: effect.effects)
        data.push_back(QString::fromStdString(printSpecificAbility(a, asn::CardType::Char)));

    auto choiceDlg = std::make_unique<ChoiceDialog>(mGame);
    choiceDlg->setData("Choose effect to perform", data);
    mChoiceDialog = std::move(choiceDlg);
}

void Player::processAbilityGain(const EventAbilityGain &event) {
    auto ability = decodeAbility(event.ability());
    auto pzone = zone(event.zone());
    auto &card = pzone->cards()[event.card_pos()];
    if (!card.cardPresent())
        return;
    card.addAbility(ability, event.ability_id());
}

void Player::processRemoveAbility(const EventRemoveAbility &event) {
    auto pzone = zone(event.zone());
    auto &card = pzone->cards()[event.card_pos()];
    if (!card.cardPresent())
        return;
    card.removeAbilityById(event.ability_id());
}

void Player::processLook(const EventLook &event) {
    if (mOpponent)
        return;

    auto effect = decodingWrapper(event.effect(), decodeLook);

    auto &activatedAbility = mAbilityList->ability(mAbilityList->activeId());
    activatedAbility.effect = effect;
    if (static_cast<asn::EffectType>(event.next_effect_type()) == asn::EffectType::MoveCard) {
        auto effect = decodingWrapper(event.next_effect(), decodeMoveCard);
        auto &moveEffect = std::get<asn::MoveCard>(activatedAbility.nextEffect);
        if (moveEffect.order == asn::Order::Any)
            zone("view")->visualItem()->setProperty("mDragEnabled", true);
    } else if (static_cast<asn::EffectType>(event.next_effect_type()) == asn::EffectType::ChooseCard)
        mAbilityList->ability(mAbilityList->activeId()).nextEffect = decodingWrapper(event.next_effect(), decodeChooseCard);

    zone("deck")->visualItem()->setProperty("mGlow", true);
    mAbilityList->activateCancel(mAbilityList->activeId(), true);
}

void Player::revealTopDeck(const EventRevealTopDeck &event) {
    zone("view")->visualItem()->setProperty("mViewMode", Game::RevealMode);

    QString code = QString::fromStdString(event.code());
    mGame->pause(600);
    createMovingCard(event.card_id(), code, "deck", 0, "view", 0, false, true, true);
}

void Player::lookTopDeck(const EventLookTopDeck &event) {
    auto view = zone("view");
    view->visualItem()->setProperty("mViewMode", Game::LookMode);

    QString code = QString::fromStdString(event.code());
    mGame->pause(400);
    createMovingCard(event.card_id(), code, "deck", 0, "view", 0, false, true, true);

    if (!mAbilityList->count())
        return;

    auto &activeAbility = mAbilityList->ability(mAbilityList->activeId());
    auto &effect = activeAbility.effect;
    if (std::holds_alternative<asn::Look>(effect)) {
        const auto &look = std::get<asn::Look>(effect);
        if (view->model().count() + 1 < look.number.value)
            zone("deck")->visualItem()->setProperty("mGlow", true);
        else
            zone("deck")->visualItem()->setProperty("mGlow", false);

        if (std::holds_alternative<asn::MoveCard>(activeAbility.nextEffect)) {
            mAbilityList->activatePlay(mAbilityList->activeId(), true, "Submit");
        } else if (std::holds_alternative<asn::ChooseCard>(activeAbility.nextEffect)) {
            const auto &chooseEffect = std::get<asn::ChooseCard>(activeAbility.nextEffect);
            if (chooseEffect.targets[0].target.type == asn::TargetType::SpecificCards) {
                const auto &spec = *chooseEffect.targets[0].target.targetSpecification;
                if (spec.number.mod == asn::NumModifier::UpTo)
                    mAbilityList->activateCancel(mAbilityList->activeId(), true);
            }
        } else if (std::holds_alternative<std::monostate>(activeAbility.nextEffect)) {
            mAbilityList->activatePlay(mAbilityList->activeId(), true, "OK");
            mAbilityList->activateCancel(mAbilityList->activeId(), false);
        }
    }
}

void Player::setCannotPlay(const EventSetCannotPlay &event) {
    if (event.hand_pos() >= mHand->model().count())
        return;

    mHand->cards()[event.hand_pos()].setCannotPlay(event.cannot_play());
    if (mHand->isPlayTiming())
        highlightPlayableCards();
}

void Player::setPlayerAttr(const EventSetPlayerAttr &event) {
    switch (event.attr()) {
    case ProtoCannotPlayBackups:
        mCannotPlayBackups = event.value();
        break;
    case ProtoCannotPlayEvents:
        mCannotPlayEvents = event.value();
        break;
    default:
        break;
    }
}

void Player::processSetCardStateTargetChoice(const EventSetCardStateTargetChoice &event) {
    if (mOpponent)
        return;

    auto effect = decodingWrapper(event.effect(), decodeChangeState);
    assert(effect.target.type == asn::TargetType::SpecificCards);

    auto &activatedAbility = mAbilityList->ability(mAbilityList->activeId());
    activatedAbility.effect = effect;

    auto checkState = [&effect](const Card &card) { return card.state() != effect.state; };

    const auto &spec = *effect.target.targetSpecification;
    highlightEligibleCards(mStage, spec.cards.cardSpecifiers, spec.mode, activatedAbility, checkState);

    auto effectText = printChangeState(effect);
    effectText[0] = std::toupper(effectText[0]);
    mGame->showText(QString::fromStdString(effectText));

    if (!event.mandatory())
        mAbilityList->activateCancel(mAbilityList->activeId(), true);
}

void Player::processSetCardBoolAttr(const EventSetCardBoolAttr &event) {
    if (mStage->model().count() <= event.card_pos())
        return;
    switch (event.attr()) {
    case ProtoCannotFrontAttack:
        mStage->cards()[event.card_pos()].setCannotFrontAttack(event.value());
        break;
    case ProtoCannotSideAttack:
        mStage->cards()[event.card_pos()].setCannotSideAttack(event.value());
        break;
    case ProtoCannotBecomeReversed:
        mStage->cards()[event.card_pos()].setCannotBecomeReversed(event.value());
        break;
    case ProtoCannotMove:
        mStage->model().setCannotMove(event.card_pos(), event.value());
        break;
    case ProtoSideAttackWithoutPenalty:
        break;
    default:
        assert(false);
    }
}

void Player::processRevealFromHand(const EventRevealFromHand &event) {
    int handPos = event.hand_pos();
    if (mHand->model().count() <= handPos)
        return;

    auto &card = mHand->cards()[handPos];
    std::string code = event.code();
    createMovingCard(card.id(), QString::fromStdString(code), "hand", handPos, "reveal", 0, false, false, true);
}
