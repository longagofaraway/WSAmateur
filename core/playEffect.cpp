#include <unordered_set>

#include "abilityEvents.pb.h"
#include "abilityCommands.pb.h"
#include "moveCommands.pb.h"

#include "abilityPlayer.h"
#include "abilityUtils.h"
#include "serverGame.h"
#include "serverPlayer.h"
#include "codecs/encode.h"

Resumable AbilityPlayer::playEffect(const asn::Effect &e, std::optional<asn::Effect> nextEffect) {
    if (!evaluateCondition(e.cond)) {
        mConditionNotMet = true;
        mPlayer->sendToBoth(EventConditionNotMet());
        co_return;
    }

    switch (e.type) {
    case asn::EffectType::NonMandatory:
        co_await playNonMandatory(std::get<asn::NonMandatory>(e.effect));
        break;
    case asn::EffectType::ChooseCard:
        co_await playChooseCard(std::get<asn::ChooseCard>(e.effect));
        break;
    case asn::EffectType::MoveCard:
        co_await playMoveCard(std::get<asn::MoveCard>(e.effect));
        break;
    case asn::EffectType::DrawCard:
        co_await playDrawCard(std::get<asn::DrawCard>(e.effect));
        break;
    case asn::EffectType::RevealCard:
        co_await playRevealCard(std::get<asn::RevealCard>(e.effect), nextEffect);
        break;
    case asn::EffectType::AttributeGain:
        playAttributeGain(std::get<asn::AttributeGain>(e.effect));
        break;
    case asn::EffectType::PayCost:
        co_await playPayCost(std::get<asn::PayCost>(e.effect));
        break;
    case asn::EffectType::SearchCard:
        co_await playSearchCard(std::get<asn::SearchCard>(e.effect));
        break;
    case asn::EffectType::Shuffle:
        playShuffle(std::get<asn::Shuffle>(e.effect));
        break;
    case asn::EffectType::AbilityGain:
        co_await playAbilityGain(std::get<asn::AbilityGain>(e.effect));
        break;
    case asn::EffectType::MoveWrToDeck:
        playMoveWrToDeck(std::get<asn::MoveWrToDeck>(e.effect));
        break;
    case asn::EffectType::ChangeState:
        co_await playChangeState(std::get<asn::ChangeState>(e.effect));
        break;
    case asn::EffectType::FlipOver:
        co_await playFlipOver(std::get<asn::FlipOver>(e.effect));
        break;
    case asn::EffectType::Backup:
        playBackup(std::get<asn::Backup>(e.effect));
        break;
    case asn::EffectType::TriggerCheckTwice:
        playTriggerCheckTwice();
        break;
    case asn::EffectType::Look:
        co_await playLook(std::get<asn::Look>(e.effect), nextEffect);
        break;
    case asn::EffectType::EarlyPlay:
        playEarlyPlay();
        break;
    case asn::EffectType::PerformEffect:
        co_await playPerformEffect(std::get<asn::PerformEffect>(e.effect));
        break;
    case asn::EffectType::DealDamage:
        co_await playDealDamage(std::get<asn::DealDamage>(e.effect));
        break;
    case asn::EffectType::SwapCards:
        co_await playSwapCards(std::get<asn::SwapCards>(e.effect));
        break;
    case asn::EffectType::CannotAttack:
        playCannotAttack(std::get<asn::CannotAttack>(e.effect));
        break;
    case asn::EffectType::CannotBecomeReversed:
        playCannotBecomeReversed(std::get<asn::CannotBecomeReversed>(e.effect));
        break;
    case asn::EffectType::OpponentAutoCannotDealDamage:
        playOpponentAutoCannotDealDamage(std::get<asn::OpponentAutoCannotDealDamage>(e.effect));
        break;
    case asn::EffectType::StockSwap:
        playStockSwap();
        break;
    case asn::EffectType::SideAttackWithoutPenalty:
        playSideAttackWithoutPenalty(std::get<asn::SideAttackWithoutPenalty>(e.effect));
        break;
    case asn::EffectType::PutOnStageRested:
        co_await playPutOnStageRested(std::get<asn::PutOnStageRested>(e.effect));
        break;
    case asn::EffectType::AddMarker:
        playAddMarker(std::get<asn::AddMarker>(e.effect));
        break;
    case asn::EffectType::RemoveMarker:
        playRemoveMarker(std::get<asn::RemoveMarker>(e.effect));
        break;
    case asn::EffectType::CannotBeChosen:
        playCannotBeChosen(std::get<asn::CannotBeChosen>(e.effect));
        break;
    case asn::EffectType::TriggerIconGain:
        playTriggerGain(std::get<asn::TriggerIconGain>(e.effect));
        break;
    case asn::EffectType::OtherEffect:
        co_await playOtherEffect(std::get<asn::OtherEffect>(e.effect));
        break;
    default:
        assert(false);
        break;
    }
    setMandatory(true);
}

namespace {
bool needNextEffect(asn::EffectType type) {
    static std::unordered_set<asn::EffectType> types {
        asn::EffectType::Look,
        asn::EffectType::RevealCard
    };
    return types.contains(type);
}
bool needToSetTargetsByServer(const asn::Target &target) {
    if (target.type != asn::TargetType::SpecificCards)
        return false;
    const auto &spec = *target.targetSpecification;
    for (const auto &cardSpec: spec.cards.cardSpecifiers) {
        if (cardSpec.type == asn::CardSpecifierType::LevelWithMultiplier)
            return true;
    }
    return false;
}
}

Resumable AbilityPlayer::playEffects(const std::vector<asn::Effect> &e) {
    for (size_t i = 0; i < e.size(); ++i) {
        if (needNextEffect(e[i].type) && (i != e.size() - 1))
            co_await playEffect(e[i], e[i + 1]);
        else
            co_await playEffect(e[i]);
        if (mConditionNotMet) {
            mConditionNotMet = false;
            break;
        }
    }
}

void AbilityPlayer::playContEffect(const asn::Effect &e) {
    switch (e.type) {
    case asn::EffectType::AttributeGain:
        playAttributeGain(std::get<asn::AttributeGain>(e.effect));
        break;
    case asn::EffectType::EarlyPlay:
        playEarlyPlay();
        break;
    case asn::EffectType::CannotPlay:
        playCannotPlay();
        break;
    case asn::EffectType::CannotUseBackupOrEvent:
        playCannotUseBackupOrEvent(std::get<asn::CannotUseBackupOrEvent>(e.effect));
        break;
    case asn::EffectType::AbilityGain:
        playTemporaryAbilityGain(std::get<asn::AbilityGain>(e.effect));
        break;
    case asn::EffectType::CannotAttack:
        playCannotAttack(std::get<asn::CannotAttack>(e.effect));
        break;
    case asn::EffectType::CannotBecomeReversed:
        playCannotBecomeReversed(std::get<asn::CannotBecomeReversed>(e.effect));
        break;
    case asn::EffectType::OpponentAutoCannotDealDamage:
        playOpponentAutoCannotDealDamage(std::get<asn::OpponentAutoCannotDealDamage>(e.effect));
        break;
    case asn::EffectType::CannotMove:
        playCannotMove(std::get<asn::CannotMove>(e.effect));
        break;
    case asn::EffectType::SideAttackWithoutPenalty:
        playSideAttackWithoutPenalty(std::get<asn::SideAttackWithoutPenalty>(e.effect));
        break;
    case asn::EffectType::CannotStand:
        playCannotStand(std::get<asn::CannotStand>(e.effect));
        break;
    case asn::EffectType::CannotBeChosen:
        playCannotBeChosen(std::get<asn::CannotBeChosen>(e.effect));
        break;
    case asn::EffectType::TriggerIconGain:
        playTriggerGain(std::get<asn::TriggerIconGain>(e.effect));
        break;
    default:
        break;
    }
}

void AbilityPlayer::applyBuff(std::vector<ServerCard *> &targets, Buff &buff) {
    if (cont()) {
        buff.source = thisCard().card;
        buff.abilityId = abilityId();
    }
    for (auto &target: targets) {
        if (cont()) {
            if (revert())
                target->buffManager()->removeContBuff(buff);
            else
                target->buffManager()->addBuff(buff);
        } else {
            target->buffManager()->addBuff(buff);
        }
    }
}

Resumable AbilityPlayer::playNonMandatory(const asn::NonMandatory &e) {
    mPerformedInFull = true;
    setMandatory(false);
    co_await playEffects(e.effect);
    setMandatory(true);
    bool youDo = !canceled();
    setCanceled(false);
    if (youDo && mPerformedInFull)
        co_await playEffects(e.ifYouDo);
    else
        co_await playEffects(e.ifYouDont);
}

Resumable AbilityPlayer::playChooseCard(const asn::ChooseCard &e, bool clearPrevious) {
    if (e.targets[0].target.type == asn::TargetType::ChosenCards)
        co_return;

    if (clearPrevious)
        clearChosenCards();

    if (findChooseTargetsAutomatically(e))
        co_return;

    std::vector<uint8_t> buf;
    encodeChooseCard(e, buf);

    EventChooseCard ev;
    ev.set_effect(buf.data(), buf.size());
    ev.set_mandatory(mandatory());
    if (needToSetTargetsByServer(e.targets[0].target)) {
        const auto &target = e.targets[0];
        auto targets = getTargets(target.target, target.placeType, target.place);
        for (const auto &t: targets) {
            ev.add_fixed_targets(t->pos());
        }
    }
    mPlayer->sendToBoth(ev);

    auto player = owner(e.executor);
    player->clearExpectedComands();
    player->addExpectedCommand(CommandChooseCard::descriptor()->name());
    // TODO: check for legitimacy of cancel
    player->addExpectedCommand(CommandCancelEffect::descriptor()->name());

    while (true) {
        GameCommand cmd;
        if (mLastCommand) {
            cmd = *mLastCommand;
            mLastCommand.reset();
        } else {
            cmd = co_await waitForCommand();
        }
        if (cmd.command().Is<CommandCancelEffect>()) {
            setCanceled(true);
            break;
        } else if (cmd.command().Is<CommandChooseCard>()) {
            CommandChooseCard chooseCmd;
            cmd.command().UnpackTo(&chooseCmd);
            // check more cases
            assert(e.targets.size() == 1);
            if (e.targets[0].target.type == asn::TargetType::SpecificCards) {
                auto &spec = e.targets[0].target.targetSpecification;
                if ((spec->number.mod == asn::NumModifier::ExactMatch &&
                    spec->number.value != chooseCmd.positions_size()) ||
                    (spec->number.mod == asn::NumModifier::AtLeast &&
                     spec->number.value > chooseCmd.positions_size()) ||
                    (spec->number.mod == asn::NumModifier::UpTo &&
                     spec->number.value < chooseCmd.positions_size()))
                continue;
            }
            //TODO: add checks
            auto relativePlayerType = protoPlayerToPlayer(chooseCmd.owner());
            auto cardOwner = relativePlayerType;
            if (e.executor == asn::Player::Opponent) {
                // revert sides
                if (cardOwner == asn::Player::Player)
                    cardOwner = asn::Player::Opponent;
                else
                    cardOwner = asn::Player::Player;
            }
            auto pzone = owner(cardOwner)->zone(chooseCmd.zone());
            if (!pzone)
                break;
            for (int i = chooseCmd.positions_size() - 1; i >= 0; --i) {
                auto card = pzone->card(chooseCmd.positions(i));
                if (!card)
                    break;
                if (relativePlayerType == asn::Player::Opponent && card->cannotBeChosen())
                    continue;
                addChosenCard(CardImprint(chooseCmd.zone(), card, card->player() != mPlayer));
                // here we assume that chosen cards from revealed/being looked at are moved after being chosen
                // it's probably better to do this at the moment the chosen card is being moved
                // by checking its unique id in selection
                if (e.targets[0].placeType == asn::PlaceType::Selection)
                    removeMentionedCard(card->id());
            }
            break;
        }
    }
    player->clearExpectedComands();
}

std::map<int, ServerCard*> AbilityPlayer::processCommandChooseCard(const CommandChooseCard &cmd) {
    //TODO: add checks
    std::map<int, ServerCard*> res;
    for (int i = cmd.positions_size() - 1; i >= 0; --i) {
        auto pzone = owner(protoPlayerToPlayer(cmd.owner()))->zone(cmd.zone());
        if (!pzone)
            break;

        auto card = pzone->card(cmd.positions(i));
        if (!card)
            break;

        res[cmd.positions(i)] = card;
    }
    return res;
}

Resumable AbilityPlayer::playDrawCard(const asn::DrawCard &e) {
    bool confirmed = mandatory();
    if (e.value.mod == asn::NumModifier::UpTo) {
        std::vector<uint8_t> buf;
        encodeDrawCard(e, buf);

        EventDrawChoice ev;
        ev.set_effect(buf.data(), buf.size());
        ev.set_mandatory(mandatory());
        mPlayer->sendToBoth(ev);

        mPlayer->clearExpectedComands();
        mPlayer->addExpectedCommand(CommandCancelEffect::descriptor()->name());
        mPlayer->addExpectedCommand(CommandPlayEffect::descriptor()->name());

        int cardsDrawn = 0;
        while (true) {
            auto cmd = co_await waitForCommand();
            if (cmd.command().Is<CommandPlayEffect>()) {
                mPlayer->moveTopDeck("hand");
                cardsDrawn++;

                if (cardsDrawn >= e.value.value)
                    break;

                mPlayer->sendToBoth(ev);
            } else if (cmd.command().Is<CommandCancelEffect>()) {
                break;
            }
        }
        mPlayer->clearExpectedComands();
        co_return;
    }
    if (!mandatory()) {
        std::vector<uint8_t> buf;
        encodeDrawCard(e, buf);

        EventDrawChoice ev;
        ev.set_effect(buf.data(), buf.size());
        ev.set_mandatory(mandatory());
        mPlayer->sendToBoth(ev);

        mPlayer->clearExpectedComands();
        mPlayer->addExpectedCommand(CommandChoice::descriptor()->name());

        while (true) {
            auto cmd = co_await waitForCommand();
            if (cmd.command().Is<CommandChoice>()) {
                CommandChoice choiceCmd;
                cmd.command().UnpackTo(&choiceCmd);
                // 0 is yes, so 'yes' will be the first on client's side
                confirmed = !choiceCmd.choice();
                break;
            }
        }
        mPlayer->clearExpectedComands();
    }
    if (!confirmed)
        co_return;
    for (int i = 0; i < e.value.value; ++i)
        mPlayer->moveTopDeck("hand");
}

void AbilityPlayer::playAttributeGain(const asn::AttributeGain &e) {
    bool positional = isPositional(e.target);
    if (e.target.type == asn::TargetType::ThisCard) {
        if (thisCard().card == nullptr || thisCard().card->zone()->name() != "stage")
            return;
    }

    auto targets = getTargets(e.target);

    int value = e.value;
    if (e.gainType == asn::ValueType::Multiplier && e.modifier->type == asn::MultiplierType::ForEach)
        value = getForEachMultiplierValue(*e.modifier) * e.value;
    for (auto card: targets) {
        if (e.gainType == asn::ValueType::Multiplier && e.modifier->type == asn::MultiplierType::TimesLevel)
            value = card->level() * e.value;
        if (cont()) {
            if (revert())
                card->buffManager()->removeContAttributeBuff(thisCard().card, abilityId(), e.type);
            else
                card->buffManager()->addContAttributeBuff(thisCard().card, abilityId(), e.type, value, positional);
        } else {
            card->buffManager()->addAttributeBuff(e.type, value, e.duration);
        }
    }
}

Resumable AbilityPlayer::playPayCost(const asn::PayCost &e) {
    mPlayer->sendToBoth(EventPayCost());

    mPlayer->clearExpectedComands();
    mPlayer->addExpectedCommand(CommandPlayEffect::descriptor()->name());
    mPlayer->addExpectedCommand(CommandCancelEffect::descriptor()->name());

    while (true) {
        auto cmd = co_await waitForCommand();
        if (cmd.command().Is<CommandCancelEffect>()) {
            setCanceled(true);
            break;
        } else if (cmd.command().Is<CommandPlayEffect>()) {
            co_await payCost();
            break;
        }
    }
    mPlayer->clearExpectedComands();

    if (!canceled())
        co_await playEffects(e.ifYouDo);

    if (canceled())
        co_await playEffects(e.ifYouDont);
    co_return;
}


Resumable AbilityPlayer::playSearchCard(const asn::SearchCard &e) {
    std::vector<uint8_t> buf;
    encodeSearchCard(e, buf);

    EventSearchCard eventPrivate;
    eventPrivate.set_effect(buf.data(), buf.size());
    EventSearchCard eventPublic(eventPrivate);
    auto pzone = mPlayer->zone(e.place.zone);
    for (int i = 0; i < pzone->count(); ++i)
        eventPrivate.add_codes(pzone->card(i)->code());
    for (int i = 0; i < pzone->count(); ++i)
        eventPublic.add_codes("");
    mPlayer->sendGameEvent(eventPrivate);
    mPlayer->game()->sendPublicEvent(eventPublic, mPlayer->id());

    mPlayer->clearExpectedComands();
    mPlayer->addExpectedCommand(CommandChooseCard::descriptor()->name());
    // TODO: check for legitimacy of cancel
    mPlayer->addExpectedCommand(CommandCancelEffect::descriptor()->name());

    while (true) {
        auto cmd = co_await waitForCommand();
        if (cmd.command().Is<CommandCancelEffect>()) {
            break;
        } else if (cmd.command().Is<CommandChooseCard>()) {
            CommandChooseCard chooseCmd;
            cmd.command().UnpackTo(&chooseCmd);

            assert(e.targets.size() == 1);
            if ((e.targets[0].number.mod == asn::NumModifier::ExactMatch &&
                e.targets[0].number.value != chooseCmd.positions_size()) ||
                (e.targets[0].number.mod == asn::NumModifier::AtLeast &&
                e.targets[0].number.value > chooseCmd.positions_size()) ||
                (e.targets[0].number.mod == asn::NumModifier::UpTo &&
                e.targets[0].number.value < chooseCmd.positions_size()))
                continue;
            //TODO: add checks
            for (int i = chooseCmd.positions_size() - 1; i >= 0; --i) {
                auto pzone = owner(protoPlayerToPlayer(chooseCmd.owner()))->zone(chooseCmd.zone());
                if (!pzone)
                    break;

                auto card = pzone->card(chooseCmd.positions(i));
                if (!card)
                    break;
                addChosenCard(CardImprint(chooseCmd.zone(), card, chooseCmd.owner() == ProtoOwner::ProtoOpponent));
            }
            break;
        }
    }
    mPlayer->clearExpectedComands();
}

void AbilityPlayer::playShuffle(const asn::Shuffle &e) {
    assert(e.owner != asn::Player::Both);
    assert(e.owner != asn::Player::NotSpecified);
    owner(e.owner)->zone(e.zone)->shuffle();
}

Resumable AbilityPlayer::playAbilityGain(const asn::AbilityGain &e) {
    auto targets = getTargets(e.target);
    if (static_cast<size_t>(e.number) < e.abilities.size()) {
        assert(e.number == 1);
        std::vector<uint8_t> buf;
        encodeAbilityGain(e, buf);

        EventAbilityChoice event;
        event.set_effect(buf.data(), buf.size());
        mPlayer->sendToBoth(event);

        mPlayer->clearExpectedComands();
        mPlayer->addExpectedCommand(CommandChoice::descriptor()->name());

        int chosenAbilityId;
        while (true) {
            auto cmd = co_await waitForCommand();
            if (cmd.command().Is<CommandChoice>()) {
                CommandChoice choiceCmd;
                cmd.command().UnpackTo(&choiceCmd);
                chosenAbilityId = choiceCmd.choice();
                if (static_cast<size_t>(chosenAbilityId) >= e.abilities.size())
                    continue;
                break;
            }
        }
        mPlayer->clearExpectedComands();

        for (auto card: targets)
            card->buffManager()->addAbilityBuff(e.abilities[chosenAbilityId], e.duration);

        co_return;
    }

    for (const auto &a: e.abilities)
        for (auto card: targets)
            card->buffManager()->addAbilityBuff(a, e.duration);
}

void AbilityPlayer::playTemporaryAbilityGain(const asn::AbilityGain &e) {
    assert(e.abilities.size() == 1);

    bool positional = false;
    if (e.target.type == asn::TargetType::SpecificCards) {
        const auto &spec = *e.target.targetSpecification;
        if (spec.mode == asn::TargetMode::InFrontOfThis || spec.mode == asn::TargetMode::BackRow
            || spec.mode == asn::TargetMode::FrontRow)
            positional = true;
    } else if (e.target.type == asn::TargetType::ThisCard) {
        if (thisCard().card == nullptr || thisCard().card->zone()->name() != "stage")
            return;
    } else {
        assert(false);
    }

    auto targets = getTargets(e.target);

    for (auto card: targets) {
        if (!revert())
            card->buffManager()->addAbilityAsContBuff(thisCard().card, abilityId(), e.abilities[0], positional);
        else
            card->buffManager()->removeAbilityAsContBuff(thisCard().card, abilityId());
    }
}

Resumable AbilityPlayer::playPerformEffect(const asn::PerformEffect &e) {
    if (static_cast<size_t>(e.numberOfEffects) < e.effects.size()) {
        assert(e.numberOfEffects == 1);
        std::vector<uint8_t> buf;
        encodePerformEffect(e, buf);

        EventEffectChoice event;
        event.set_effect(buf.data(), buf.size());
        mPlayer->sendToBoth(event);

        mPlayer->clearExpectedComands();
        mPlayer->addExpectedCommand(CommandChoice::descriptor()->name());

        int chosenEffectId;
        while (true) {
            auto cmd = co_await waitForCommand();
            if (cmd.command().Is<CommandChoice>()) {
                CommandChoice choiceCmd;
                cmd.command().UnpackTo(&choiceCmd);
                chosenEffectId = choiceCmd.choice();
                if (static_cast<size_t>(chosenEffectId) >= e.effects.size())
                    continue;
                break;
            }
        }
        mPlayer->clearExpectedComands();

        co_await playEventAbility(e.effects[chosenEffectId]);

        co_return;
    }

    for (int i = 0; i < e.numberOfTimes; ++i)
        for (const auto &a: e.effects)
            co_await playEventAbility(a);
}

void AbilityPlayer::playMoveWrToDeck(const asn::MoveWrToDeck &e) {
    if (e.executor == asn::Player::Player || e.executor == asn::Player::Both) {
        mPlayer->moveWrToDeck();
        mPlayer->sendToBoth(EventRefresh());
    }
    if (e.executor == asn::Player::Opponent || e.executor == asn::Player::Both) {
        auto opponent = mPlayer->game()->opponentOfPlayer(mPlayer->id());
        opponent->moveWrToDeck();
        opponent->sendToBoth(EventRefresh());
    }
}

Resumable AbilityPlayer::playChangeState(const asn::ChangeState &e) {
    if (e.target.type == asn::TargetType::ThisCard) {
        if (thisCard().zone != thisCard().card->zone()->name())
            co_return;
        mPlayer->setCardState(thisCard().card, e.state);
    } else if (e.target.type == asn::TargetType::ChosenCards) {
        for (const auto &card: chosenCards()) {
            auto player = owner(card.opponent ? asn::Player::Opponent : asn::Player::Player);
            player->setCardState(card.card, e.state);
        }
    } else if (e.target.type == asn::TargetType::BattleOpponent ||
               e.target.type == asn::TargetType::OppositeThis) {
        auto targets = getTargets(e.target);
        std::erase_if(targets, [&e](auto card) { return card->state() == e.state; });
        if (targets.empty())
            co_return;
        auto card = targets.front();

        bool confirmed = true;
        if (!mandatory()) {
            std::vector<uint8_t> buf;
            encodeChangeState(e, buf);

            EventSetCardStateChoice ev;
            ev.set_effect(buf.data(), buf.size());
            ev.set_mandatory(mandatory());

            mPlayer->clearExpectedComands();
            mPlayer->addExpectedCommand(CommandChoice::descriptor()->name());
            mPlayer->sendToBoth(ev);

            while (true) {
                GameCommand cmd = co_await waitForCommand();
                if (cmd.command().Is<CommandChoice>()) {
                    CommandChoice choiceCmd;
                    cmd.command().UnpackTo(&choiceCmd);
                    // 0 is yes, so 'yes' will be the first on client's side
                    confirmed = !choiceCmd.choice();
                    break;
                }
            }
            mPlayer->clearExpectedComands();
        }

        if (confirmed)
            card->player()->setCardState(card, e.state);
    } else if (e.target.type == asn::TargetType::SpecificCards) {
        const auto &spec = *e.target.targetSpecification;

        auto targets = getTargets(e.target);
        std::erase_if(targets, [&e](auto card) { return card->state() == e.state; });
        assert(spec.number.mod == asn::NumModifier::ExactMatch);
        if (spec.number.value > targets.size())
            co_return;

        if (!mandatory() || spec.number.value < targets.size()) {
            std::vector<uint8_t> buf;
            encodeChangeState(e, buf);

            EventSetCardStateTargetChoice ev;
            ev.set_effect(buf.data(), buf.size());
            ev.set_mandatory(mandatory());

            mPlayer->clearExpectedComands();
            mPlayer->addExpectedCommand(CommandChooseCard::descriptor()->name());
            mPlayer->addExpectedCommand(CommandCancelEffect::descriptor()->name());

            bool canceled = false;
            for (int i = 0; i < spec.number.value; ++i) {
                mPlayer->sendToBoth(ev);
                while (true) {
                    GameCommand cmd = co_await waitForCommand();
                    if (cmd.command().Is<CommandCancelEffect>() && !mandatory()) {
                        setCanceled(true);
                        canceled = true;
                        break;
                    } else if (cmd.command().Is<CommandChooseCard>()) {
                        CommandChooseCard chooseCmd;
                        cmd.command().UnpackTo(&chooseCmd);
                        auto chosenCards = processCommandChooseCard(chooseCmd);
                        if (chosenCards.size() != 1)
                            continue;

                        auto card = chosenCards.begin()->second;
                        if (card->state() == e.state)
                            continue;

                        auto player = card->player();
                        player->setCardState(card, e.state);
                        break;
                    }
                }

                if (canceled)
                    break;
            }

            mPlayer->clearExpectedComands();
        } else {
            for (auto target: targets) {
                target->player()->setCardState(target, e.state);
            }
        }
    } else {
        assert(false);
    }
}

Resumable AbilityPlayer::playFlipOver(const asn::FlipOver &e) {
    assert(e.number.mod == asn::NumModifier::ExactMatch);
    for (int i = 0; i < e.number.value; ++i)
        mPlayer->moveTopDeck("res");

    auto res = mPlayer->zone("res");
    int count = 0, climaxCount = 0;
    int cardCount = res->count();
    for (int i = 0; i < cardCount; ++i) {
        auto card = res->card(res->count() - 1);
        if (checkCard(e.forEach.cardSpecifiers, *card))
            ++count;
        if (card->type() == asn::CardType::Climax)
            ++climaxCount;
        mPlayer->moveCard("res", res->count() - 1, "wr");
    }
    if (climaxCount)
        owner(asn::Player::Opponent)->checkOtherTrigger("KGL/S79-016");

    for (int i = 0; i < count; ++i)
        co_await playEffects(e.effect);
}

void AbilityPlayer::playBackup(const asn::Backup &e) {
    auto opponent = mPlayer->getOpponent();
    auto charInBattle = opponent->oppositeCard(opponent->attackingCard());
    if (charInBattle)
        charInBattle->buffManager()->addAttributeBuff(asn::AttributeType::Power, e.power, 1);
    mPlayer->checkOnBackup(thisCard().card);
}

void AbilityPlayer::playTriggerCheckTwice() {
    thisCard().card->setTriggerCheckTwice(true);
}

void AbilityPlayer::playEarlyPlay() {
    if (revert())
        thisCard().card->buffManager()->removeContAttributeBuff(thisCard().card, abilityId(), asn::AttributeType::Level);
    else
        thisCard().card->buffManager()->addContAttributeBuff(thisCard().card, abilityId(), asn::AttributeType::Level, -1);
}

void AbilityPlayer::playCannotPlay() {
    thisCard().card->setCannotPlay(!revert());

    EventSetCannotPlay ev;
    ev.set_hand_pos(thisCard().card->pos());
    ev.set_cannot_play(thisCard().card->cannotPlay());
    mPlayer->sendGameEvent(ev);
}

void AbilityPlayer::setCannotPlayBackupOrEvent(ServerPlayer *player, asn::BackupOrEvent type) {
    assert(cont());
    if (!cont())
        return;
    bool forbidEvents = !revert() && (type == asn::BackupOrEvent::Event || type == asn::BackupOrEvent::Both);
    bool forbidBackups = !revert() && (type == asn::BackupOrEvent::Backup || type == asn::BackupOrEvent::Both);
    if (revert()) {
        if (!forbidEvents)
            player->buffManager()->removeContAttrChange(thisCard().card, abilityId(), PlayerAttrType::CannotPlayEvents);
        if (!forbidBackups)
            player->buffManager()->removeContAttrChange(thisCard().card, abilityId(), PlayerAttrType::CannotPlayBackups);
    } else {
        if (forbidEvents)
            player->buffManager()->addContAttrChange(thisCard().card, abilityId(), PlayerAttrType::CannotPlayEvents);
        if (forbidBackups)
            player->buffManager()->addContAttrChange(thisCard().card, abilityId(), PlayerAttrType::CannotPlayBackups);
    }
}

void AbilityPlayer::playCannotUseBackupOrEvent(const asn::CannotUseBackupOrEvent &e) {
    if (e.player == asn::Player::Player || e.player == asn::Player::Both)
        setCannotPlayBackupOrEvent(mPlayer, e.what);
    if (e.player == asn::Player::Opponent || e.player == asn::Player::Both)
        setCannotPlayBackupOrEvent(mPlayer->getOpponent(), e.what);
}

Resumable AbilityPlayer::playDealDamage(const asn::DealDamage &e) {
    if (mPlayer->charAutoCannotDealDamage())
        co_return;

    int damage;
    if (e.damageType == asn::ValueType::Multiplier) {
        if (e.modifier->type == asn::MultiplierType::ForEach)
            damage = getForEachMultiplierValue(*e.modifier) * e.damage;
        else if (e.modifier->type == asn::MultiplierType::AddLevel)
            damage = getAddLevelMultiplierValue(*e.modifier) + e.damage;
        else if (e.modifier->type == asn::MultiplierType::AddTriggerNumber)
            damage = getTriggerNumberMultiplierValue(*e.modifier) + e.damage;
        else if (e.modifier->type == asn::MultiplierType::PreviousDamage)
            damage = thisCard().card->previousDamage();
    } else
        damage = e.damage;

    co_await mPlayer->getOpponent()->takeDamage(damage, thisCard().card);
}

Resumable AbilityPlayer::playSwapCards(const asn::SwapCards &e) {
    co_await playChooseCard(e.first);
    co_await playChooseCard(e.second, false);

    if (chosenCards().size() != 2)
        co_return;

    auto card1 = chosenCards()[0].card;
    auto card2 = chosenCards()[1].card;

    if (card1->id() == card2->id())
        co_return;

    assert(card1->zone()->name() != "stage" && card2->zone()->name() != "stage");

    auto card1Zone = card1->zone()->name();
    auto card1Pos = card1->pos();
    auto card2Pos = card2->pos();
    auto player = card1->player();
    player->moveCard(card1Zone, card1->pos(), card2->zone()->name(), card2->pos());
    player->moveCard(card2->zone()->name(), card2->pos(), card1Zone, card1Pos);
}

void AbilityPlayer::playCannotAttack(const asn::CannotAttack &e) {
    bool positional = isPositional(e.target);
    if (e.target.type == asn::TargetType::ThisCard) {
        if (thisCard().card == nullptr || thisCard().card->zone()->name() != "stage")
            return;
    }

    auto targets = getTargets(e.target);
    for (auto target: targets) {
        auto attr = e.type == asn::AttackType::Frontal ? BoolAttributeType::CannotFrontAttack
                                                       : BoolAttributeType::CannotSideAttack;

        if (cont()) {
            if (revert())
                target->buffManager()->removeContBoolAttrChange(thisCard().card, abilityId(), attr);
            else
                target->buffManager()->addContBoolAttrChange(thisCard().card, abilityId(), attr, positional);
        } else {
            target->buffManager()->addBoolAttrChange(attr, e.duration);
        }
    }
}

void AbilityPlayer::playCannotBecomeReversed(const asn::CannotBecomeReversed &e) {
    bool positional = isPositional(e.target);
    if (e.target.type == asn::TargetType::ThisCard) {
        if (thisCard().card == nullptr || thisCard().card->zone()->name() != "stage")
            return;
    }

    auto attr = BoolAttributeType::CannotBecomeReversed;
    auto targets = getTargets(e.target);
    for (auto target: targets) {
        if (cont()) {
            if (revert())
                target->buffManager()->removeContBoolAttrChange(thisCard().card, abilityId(), attr);
            else
                target->buffManager()->addContBoolAttrChange(thisCard().card, abilityId(), attr, positional);
        } else {
            target->buffManager()->addBoolAttrChange(attr, e.duration);
        }
    }
}

void AbilityPlayer::playCannotBeChosen(const asn::CannotBeChosen &e) {
    bool positional = isPositional(e.target);
    if (e.target.type == asn::TargetType::ThisCard) {
        if (thisCard().card == nullptr || thisCard().card->zone()->name() != "stage")
            return;
    }

    auto attr = BoolAttributeType::CannotBeChosen;
    auto targets = getTargets(e.target);
    for (auto target: targets) {
        if (cont()) {
            if (revert())
                target->buffManager()->removeContBoolAttrChange(thisCard().card, abilityId(), attr);
            else
                target->buffManager()->addContBoolAttrChange(thisCard().card, abilityId(), attr, positional);
        } else {
            target->buffManager()->addBoolAttrChange(attr, e.duration);
        }
    }
}

void AbilityPlayer::playOpponentAutoCannotDealDamage(const asn::OpponentAutoCannotDealDamage &e) {
    auto attr = PlayerAttrType::CharAutoCannotDealDamage;
    auto buffManager = mPlayer->getOpponent()->buffManager();
    if (cont()) {
        if (revert())
            buffManager->removeContAttrChange(thisCard().card, abilityId(), attr);
        else
            buffManager->addContAttrChange(thisCard().card, abilityId(), attr);
    } else {
        buffManager->addAttrChange(attr, e.duration);
    }
}

void AbilityPlayer::playStockSwap() {
    auto opponent = mPlayer->getOpponent();
    auto stock = opponent->zone("stock");
    int stockCount = stock->count();
    while (stock->count())
        opponent->moveCard("stock", stock->count() - 1, "wr");

    for (int i = 0; i < stockCount; ++i)
        opponent->moveTopDeck("stock");
}

void AbilityPlayer::playCannotMove(const asn::CannotMove &e) {
    bool positional = isPositional(e.target);

    auto targets = getTargets(e.target);
    for (auto &target: targets) {
        if (cont()) {
            if (revert())
                target->buffManager()->removeContBoolAttrChange(thisCard().card, abilityId(), BoolAttributeType::CannotMove);
            else
                target->buffManager()->addContBoolAttrChange(thisCard().card, abilityId(), BoolAttributeType::CannotMove, positional);
        } else {
            target->buffManager()->addBoolAttrChange(BoolAttributeType::CannotMove, e.duration);
        }
    }
}

void AbilityPlayer::playCannotStand(const asn::CannotStand &e) {
    bool positional = isPositional(e.target);

    auto targets = getTargets(e.target);
    for (auto &target: targets) {
        if (cont()) {
            if (revert())
                target->buffManager()->removeContBoolAttrChange(thisCard().card, abilityId(), BoolAttributeType::CannotStand);
            else
                target->buffManager()->addContBoolAttrChange(thisCard().card, abilityId(), BoolAttributeType::CannotStand, positional);
        } else {
            target->buffManager()->addBoolAttrChange(BoolAttributeType::CannotStand, e.duration);
        }
    }
}

void AbilityPlayer::playSideAttackWithoutPenalty(const asn::SideAttackWithoutPenalty &e) {
    auto attr = BoolAttributeType::SideAttackWithoutPenalty;
    bool positional = isPositional(e.target);

    auto targets = getTargets(e.target);
    for (auto &target: targets) {
        if (cont()) {
            if (revert())
                target->buffManager()->removeContBoolAttrChange(thisCard().card, abilityId(), attr);
            else
                target->buffManager()->addContBoolAttrChange(thisCard().card, abilityId(), attr, positional);
        } else {
            target->buffManager()->addBoolAttrChange(attr, e.duration);
        }
    }
}

Resumable AbilityPlayer::playPutOnStageRested(const asn::PutOnStageRested &e) {
    asn::MoveCard effect;
    effect.executor = asn::Player::Player;
    effect.target = e.target;
    effect.from = e.from;
    effect.order = asn::Order::NotSpecified;
    effect.to.emplace_back(asn::Place{e.to, asn::Zone::Stage, asn::Player::Player});

    co_await playMoveCard(effect);

    // notice: using lastMovedCard might not always work, but we assume it will
    for (auto &target: lastMovedCards())
        target.card->player()->setCardState(target.card, asn::State::Rested);

    co_return;
}

void AbilityPlayer::playAddMarker(const asn::AddMarker &e) {
    if ((e.target.type == asn::TargetType::ChosenCards && chosenCards().empty()) ||
        ((e.target.type == asn::TargetType::MentionedCards || e.target.type == asn::TargetType::RestOfTheCards) &&
            mentionedCards().empty()) ||
        (e.target.type == asn::TargetType::ThisCard && thisCard().zone != thisCard().card->zone()->name()))
        return;

    if (e.target.type == asn::TargetType::SpecificCards && e.from.pos == asn::Position::NotSpecified
        && e.target.targetSpecification->mode != asn::TargetMode::All) {
        // todo: implement choice of marker
        assert(false);
    }

    // TODO: add choice of target stage cards
    auto targetStageCards = getTargets(e.destination);
    if (targetStageCards.empty())
        return;
    auto targetStageCard = targetStageCards.front();

    auto player = owner(e.from.owner);
    auto pzone = player->zone(e.from.zone);
    if (e.target.type == asn::TargetType::SpecificCards && e.from.pos == asn::Position::Top &&
        e.from.zone == asn::Zone::Deck) {
        const auto &spec = *e.target.targetSpecification;
        assert(spec.number.mod == asn::NumModifier::ExactMatch);

        for (int i = 0; i < spec.number.value; ++i) {
            auto card = pzone->topCard();
            if (!card)
                break;

            player->addMarker(pzone, pzone->count() - 1, targetStageCard->pos(), e.orientation);
            if (pzone->count() == 0)
                player->refresh();
        }
        return;
    }

    // TODO: won't work with TargetType::SpecificCards
    auto targets = getTargets(e.target);
    std::sort(targets.begin(), targets.end(), [](const ServerCard *card1, const ServerCard * card2) {
        return card1->pos() > card2->pos();
    });
    for (auto target: targets) {
        player->addMarker(pzone, target->pos(), targetStageCard->pos(), e.orientation);

        if (e.from.zone == asn::Zone::Deck && pzone->count() == 0)
            player->refresh();
    }
}

void AbilityPlayer::playRemoveMarker(const asn::RemoveMarker &e) {
    auto targetStageCards = getTargets(e.markerBearer);
    if (targetStageCards.empty())
        return;

    // no choice for now
    assert(targetStageCards.size() == 1);
    auto markerBearer = targetStageCards.front();

    if (e.targetMarker.type == asn::TargetType::SpecificCards) {
        const auto &spec = *e.targetMarker.targetSpecification;
        for (int i = 0; i < spec.number.value; ++i)
            mPlayer->removeTopMarker(markerBearer, e.place);
    }
}

void AbilityPlayer::playTriggerGain(const asn::TriggerIconGain &e) {
    auto targets = getTargetsFromAllZones(e.target);

    TriggerIconBuff buff(e.triggerIcon, e.duration);
    applyBuff(targets, buff);
}

Resumable AbilityPlayer::playOtherEffect(const asn::OtherEffect &e) {
    auto key = e.cardCode + '-' + std::to_string(e.effectId);
    if (key == "KGL/S79-020-3") {
        co_await playS79_20();
    }
}

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
        a.gainType = asn::ValueType::Raw;
        a.value = 5000;
        a.duration = 1;

        playAttributeGain(a);
    }
}

