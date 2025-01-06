#include "player.h"

#include <QTimer>

#include "abilityEvents.pb.h"
#include "abilityCommands.pb.h"
#include "gameEvent.pb.h"
#include "moveEvents.pb.h"

#include "abilityUtils.h"
#include "cardZone.h"
#include "commonCardZone.h"
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
    if (effect.targets.size() < 1 ||
        effect.targets[0].target.type != asn::TargetType::SpecificCards) {
        assert(false);
        return;
    }

    const auto &spec = *effect.targets[0].target.targetSpecification;
    bool mandatory = event.mandatory();
    if (spec.number.mod == asn::NumModifier::UpTo)
        mandatory = false;

    int eligibleCardsNum = highlightCardsFromEvent(event, effect);
    if (eligibleCardsNum)
        mAbilityList->ability(mAbilityList->activeId()).effect = effect;

    std::vector<asn::Place> places;
    for (const auto &target: effect.targets) {
        if (target.placeType == asn::PlaceType::SpecificPlace && target.place)
            places.push_back(target.place.value());
    }
    processChooseCardInternal(eligibleCardsNum, places,
                              mandatory, effect.executor);
}

void Player::processSearchCard(const EventSearchCard &event) {
    if (mOpponent)
        return;

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
        QTimer::singleShot(800, this, [this]() { mDeckView->hide(); });
    }
}

void Player::processMoveChoice(const EventMoveChoice &event) {
    if (mOpponent)
        return;

    std::string effectText;
    ResetPrintState(event.print_context().mentioned_cards_count(),
                    event.print_context().last_moved_cards_count());
    if (event.effect_type() == static_cast<int>(asn::EffectType::RemoveMarker)) {
        auto effect = decodingWrapper(event.effect(), decodeRemoveMarker);
        effectText = printRemoveMarker(effect) + '?';
    } else if (event.effect_type() == static_cast<int>(asn::EffectType::MoveCard)) {
        auto effect = decodingWrapper(event.effect(), decodeMoveCard);
        assert(effect.executor == asn::Player::Player);
        effectText = printMoveCard(effect) + '?';
    } else if (event.effect_type() == static_cast<int>(asn::EffectType::AddMarker)) {
        auto effect = decodingWrapper(event.effect(), decodeAddMarker);
        effectText = printAddMarker(effect) + '?';
    }

    highlightActiveAbilityCharacter();
    if (!effectText.empty())
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
        for (const auto &to: effect.to)
            data.push_back(placeToReadableString(to));

        auto choiceDlg = std::make_unique<ChoiceDialog>(mGame);
        choiceDlg->setData("Choose where to put the card", data);
        mChoiceDialog = std::move(choiceDlg);
        return;
    }
}

void Player::processMoveDestinationIndexChoice(const EventMoveDestinationIndexChoice &event) {
    auto &a = mAbilityList->ability(mAbilityList->activeId());
    asn::Place place{};
    if (static_cast<asn::EffectType>(event.effect_type()) == asn::EffectType::MoveCard) {
        auto effect = decodingWrapper(event.effect(), decodeMoveCard);
        if ((mOpponent && effect.executor == asn::Player::Player) ||
            (!mOpponent && effect.executor == asn::Player::Opponent)) {
            return;
        }
        assert(effect.to.size() == 1);
        a.effect = effect;
        place = effect.to.at(0);
    } else if (static_cast<asn::EffectType>(event.effect_type()) == asn::EffectType::RemoveMarker) {
        if (mOpponent) {
            return;
        }
        auto effect = decodingWrapper(event.effect(), decodeRemoveMarker);
        a.effect = effect;
        place = effect.place;
    } else {
        qWarning() << "met unknown effect in dest index choice";
        return;
    }

    a.choiceType = ChoiceType::DestinationIndex;
    auto player = place.owner == asn::Player::Opponent ? getOpponent() : this;
    auto stage = player->zone("stage");
    if (place.pos == asn::Position::EmptySlotFrontRow) {
        for (int i = 0; i < 3; ++i) {
            if (!stage->cards()[i].cardPresent())
                stage->model().setGlow(i, true);
        }
    } if (place.pos == asn::Position::EmptySlotBackRow) {
        // highlight all row, because otherwise the move will be either auto-performed or auto-declined
        stage->model().setGlow(3, true);
        stage->model().setGlow(4, true);
    } else if (place.pos == asn::Position::EmptySlot) {
        for (int i = 0; i < stage->model().count(); ++i) {
            if (!stage->cards()[i].cardPresent())
                stage->model().setGlow(i, true);
        }
    } else if (place.zone == asn::Zone::Stage && place.pos == asn::Position::NotSpecified) {
        highlightAllCards(stage);
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
        activeAbility().effect = effect;
    processChooseCardInternal(eligibleCount, {effect.from}, event.mandatory(), effect.executor);
}

void Player::processDrawChoice(const EventDrawChoice &event) {
    if (mOpponent)
        return;

    auto effect = decodingWrapper(event.effect(), decodeDrawCard);

    auto header = printDrawCard(effect);
    header[0] = std::toupper(header[0]);
    if (effect.value.mod == asn::NumModifier::UpTo) {
        mGame->showText(QString::fromStdString(header));
        activeAbility().effect = effect;
        zone("deck")->visualItem()->setProperty("mGlow", true);
        mAbilityList->activateCancel(mAbilityList->activeId(), true);
    } else if (!event.mandatory()) {
        header.push_back('?');
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

        highlightActiveAbilityCharacter();
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

    highlightActiveAbilityCharacter();
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

    auto lookEffect = decodingWrapper(event.effect(), decodeLook);
    activeAbility().effect = lookEffect;
    processLookRevealCommon(static_cast<asn::EffectType>(event.next_effect_type()),
                            event.next_effect(), lookEffect.place.owner == asn::Player::Opponent);
}

void Player::processReveal(const EventReveal &event) {
    if (mOpponent)
        return;

    activeAbility().effect = decodingWrapper(event.effect(), decodeRevealCard);
    processLookRevealCommon(static_cast<asn::EffectType>(event.next_effect_type()), event.next_effect());
}

void Player::processLookRevealCommon(
        asn::EffectType nextEffectType,
        const std::string &nextEffectBuf,
        bool opponent) {
    Player *owner = getOpponent(opponent);

    if (nextEffectType == asn::EffectType::MoveCard) {
        auto nextEffect = decodingWrapper(nextEffectBuf, decodeMoveCard);
        if (nextEffect.order == asn::Order::Any)
            owner->zone("view")->visualItem()->setProperty("mDragEnabled", true);
        activeAbility().nextEffect = nextEffect;
    } else if (nextEffectType == asn::EffectType::ChooseCard) {
        auto choose = decodingWrapper(nextEffectBuf, decodeChooseCard);
        if (choose.targets[0].placeType == asn::PlaceType::Selection)
            activeAbility().nextEffect = choose;
    }

    owner->zone("deck")->visualItem()->setProperty("mGlow", true);
    if (opponent)
        owner->zone("view")->visualItem()->setProperty("overlapsWithAbilities", true);
    mAbilityList->activateCancel(mAbilityList->activeId(), true);
}

void Player::processLookRevealNextCard(asn::EffectType type, bool isOwnerOpponent) {
    if (mOpponent || !mAbilityList->count())
        return;

    auto player = getOpponent(isOwnerOpponent);
    auto view = player->zone("view");

    auto &activeAbility_ = activeAbility();
    auto &effect = activeAbility_.effect;

    if (!std::holds_alternative<asn::Look>(effect) &&
        !std::holds_alternative<asn::RevealCard>(effect))
        return;

    int numCards = 0;
    if (std::holds_alternative<asn::Look>(effect)) {
        const auto &look = std::get<asn::Look>(effect);
        numCards = look.number.value;
        if (look.valueType == asn::ValueType::Multiplier && look.multiplier->type == asn::MultiplierType::ForEach) {
            numCards = look.number.value * getForEachMultiplierValue(this, activeAbility_.cardId, look.multiplier.value());
        }
    } else if (std::holds_alternative<asn::RevealCard>(effect)) {
        const auto &reveal = std::get<asn::RevealCard>(effect);
        numCards = reveal.number.value;
    }

    auto deck = player->zone("deck");

    if (view->model().count() + 1 < numCards && view->model().count() + 1 < deck->model().count())
        deck->visualItem()->setProperty("mGlow", true);
    else
        deck->visualItem()->setProperty("mGlow", false);

    if (std::holds_alternative<asn::MoveCard>(activeAbility_.nextEffect)) {
        mAbilityList->activatePlay(mAbilityList->activeId(), true, "Submit");
    } else if (std::holds_alternative<asn::ChooseCard>(activeAbility_.nextEffect)) {
        const auto &chooseEffect = std::get<asn::ChooseCard>(activeAbility_.nextEffect);
        if (chooseEffect.targets[0].target.type == asn::TargetType::SpecificCards) {
            const auto &spec = *chooseEffect.targets[0].target.targetSpecification;
            if (spec.number.mod == asn::NumModifier::UpTo)
                mAbilityList->activateCancel(mAbilityList->activeId(), true);
        }
    } else if (std::holds_alternative<std::monostate>(activeAbility_.nextEffect)) {
        mAbilityList->activatePlay(mAbilityList->activeId(), true, "OK");
        mAbilityList->activateCancel(mAbilityList->activeId(), false);
    }
}

void Player::revealTopDeck(const EventRevealTopDeck &event) {
    zone("view")->visualItem()->setProperty("mViewMode", Game::RevealMode);

    QString code = QString::fromStdString(event.code());

    bool dontFinishAction{true};
    if (mAbilityList->count()) {
        auto &activeAbility_ = activeAbility();
        auto &effect = activeAbility_.effect;

        if (!std::holds_alternative<asn::Look>(effect) &&
            !std::holds_alternative<asn::RevealCard>(effect))
            dontFinishAction = false;
    }
    if (dontFinishAction) {
        mGame->pause(600);
    } else {
        mGame->delayNextEvent(event.GetMetadata().descriptor->full_name(), 600);
    }
    createMovingCard(event.card_id(), code, "deck", 0, "view",
                     MovingParams{.targetPos = -1,
                                  .dontFinishAction = dontFinishAction,
                                  .noDelete = true});

    processLookRevealNextCard(asn::EffectType::RevealCard);
}

void Player::lookTopDeck(const EventLookTopDeck &event) {
    auto player = getOpponent(event.is_opponent());
    auto view = player->zone("view");
    view->visualItem()->setProperty("mViewMode", Game::LookMode);

    QString code = QString::fromStdString(event.code());
    mGame->pause(400);
    player->createMovingCard(event.card_id(), code, "deck", 0, "view",
                             MovingParams{.targetPos = -1,
                                          .dontFinishAction = true,
                                          .noDelete = true});


    processLookRevealNextCard(asn::EffectType::Look, event.is_opponent());
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

void Player::processSetCardStateChoice(const EventSetCardStateChoice &event) {
    if (mOpponent)
        return;

    auto effect = decodingWrapper(event.effect(), decodeChangeState);

    auto &activatedAbility = mAbilityList->ability(mAbilityList->activeId());
    activatedAbility.effect = effect;

    highlightActiveAbilityCharacter();
    auto effectText = printChangeState(effect) + '?';
    effectText[0] = std::toupper(effectText[0]);
    std::vector<QString> data { "Yes", "No" };
    auto choiceDlg = std::make_unique<ChoiceDialog>(mGame);
    choiceDlg->setData(QString::fromStdString(effectText), data);
    mChoiceDialog = std::move(choiceDlg);
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
    highlightEligibleCards(mStage, spec.cards.cardSpecifiers, spec.mode,
                           activatedAbility, checkState, false);

    auto effectText = printChangeState(effect);
    effectText[0] = std::toupper(effectText[0]);
    mGame->showText(QString::fromStdString(effectText));

    if (!event.mandatory())
        mAbilityList->activateCancel(mAbilityList->activeId(), true);
}

void Player::processSetCardBoolAttr(const EventSetCardBoolAttr &event) {
    auto pzone = zone(event.zone());
    if (!pzone) {
        qWarning() << "no zone in EventSetCardBoolAttr";
        return;
    }
    if (pzone->model().count() <= event.card_pos()) {
        qWarning() << "Failed to set bool attr " << event.attr()
                   << " to card at pos " << event.card_pos();
        return;
    }
    switch (event.attr()) {
    case ProtoCannotFrontAttack:
        pzone->cards()[event.card_pos()].setCannotFrontAttack(event.value());
        break;
    case ProtoCannotSideAttack:
        pzone->cards()[event.card_pos()].setCannotSideAttack(event.value());
        break;
    case ProtoCannotBecomeReversed:
        pzone->cards()[event.card_pos()].setCannotBecomeReversed(event.value());
        break;
    case ProtoCannotMove:
        pzone->model().setCannotMove(event.card_pos(), event.value());
        break;
    case ProtoCannotBeChosen:
        pzone->cards()[event.card_pos()].setCannotBeChosen(event.value());
        break;
    case ProtoPlayWithoutColorRequirement: {
        pzone->cards()[event.card_pos()].setCanPlayWoColorReq(event.value());
        if (mHand->isPlayTiming())
            highlightPlayableCards();
        break;
    }
    case ProtoSideAttackWithoutPenalty:
    case ProtoCannotStand:
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
    createMovingCard(card.id(), QString::fromStdString(code), "hand", handPos, "reveal",
                     MovingParams{.targetPos = -1,
                                  .noDelete = true});

}

void Player::processRuleActionChoice() {
    std::vector<QString> data;
    data.emplace_back("Refresh");
    data.emplace_back("Level up");

    auto choiceDlg = std::make_unique<ChoiceDialog>(mGame);
    choiceDlg->setData("Choose what to perform first", data);
    mChoiceDialog = std::move(choiceDlg);
}

void Player::processPlayableCards(const EventPlayableCards &event) {
    const auto &cards = mHand->cards();
    for (int i = 0; i < event.hand_pos_size(); ++i) {
        if (static_cast<size_t>(event.hand_pos(i)) > cards.size()) {
            qWarning() << "EventPlayableCards out of bounds card";
            break;
        }
        mHand->model().setGlow(event.hand_pos(i), true);
    }
}

void Player::processTextChoice(const EventTextChoice &event) {
    std::vector<QString> data;
    for (int i = 0; i < event.choices_size(); ++i)
        data.push_back(QString::fromStdString(event.choices(i)));
    auto choiceDlg = std::make_unique<ChoiceDialog>(mGame);
    choiceDlg->setData(QString::fromStdString(event.header()), data);
    mChoiceDialog = std::move(choiceDlg);
    highlightActiveAbilityCharacter();
}

void Player::processConfirmationRequest() {
    if (mOpponent || !mAbilityList->count())
        return;

    mAbilityList->activatePlay(mAbilityList->activeId(), true, "OK");
    auto &activatedAbility = mAbilityList->ability(mAbilityList->activeId());
    activatedAbility.awaitingConfirmation = true;
}

void Player::processStartMoveLog() {
    mMoveLog = std::make_unique<CommonCardZone>(this, mGame, "view");

    QMetaObject::invokeMethod(mMoveLog->visualItem(), "centerView");
    mMoveLog->visualItem()->setProperty("mViewMode", Game::LastMovedCardsMode);
}

void Player::processEndMoveLog(const EventEndMoveLog &event) {
    if (event.is_confirmed()) {
        deleteMoveLog();
        return;
    }

    if (mOpponent || !mAbilityList->count())
        return;

    mAbilityList->activatePlay(mAbilityList->activeId(), true, "OK");
    auto &activatedAbility = mAbilityList->ability(mAbilityList->activeId());
    activatedAbility.awaitingConfirmation = true;
}
