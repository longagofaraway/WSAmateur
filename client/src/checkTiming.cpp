#include "player.h"

#include "abilityCommands.pb.h"
#include "abilityEvents.pb.h"

#include "cardDatabase.h"
#include "game.h"
#include "globalAbilities/globalAbilities.h"
#include "hand.h"
#include "stage.h"
#include "utils.h"

void Player::stopUiInteractions() {
    if (mOpponent)
        return;

    switch (mGame->phase()) {
    case asn::Phase::MainPhase:
        if (mActivePlayer) {
            mHand->endPlayTiming();
            mStage->endMainPhase();
            mGame->pauseMainPhase();
        }
        break;
    case asn::Phase::AttackPhase:
    case asn::Phase::CounterStep:
    case asn::Phase::DamageStep: {
        Player *activePlayer = mActivePlayer ? this : mGame->opponent();
        activePlayer->mStage->unhighlightAttacker();
        break;
    }
    default:
        break;
    }
    mResolvingAbilities = true;
}

void Player::restoreUiState() {
    if (mOpponent)
        return;

    switch (mGame->phase()) {
    case asn::Phase::MainPhase: {
        if (mActivePlayer) {
            highlightPlayableCards();
            mHand->playTiming();
            mStage->mainPhase();
            mGame->mainPhase();
        }
        break;
    }
    case asn::Phase::AttackPhase:
    case asn::Phase::CounterStep: {
        Player *activePlayer = mActivePlayer ? this : mGame->opponent();
        const auto &card = activePlayer->mStage->cards()[activePlayer->mAttackingPos];
        if (card.cardPresent() && card.state() == asn::State::Rested)
            activePlayer->mStage->model().setSelected(activePlayer->mAttackingPos, true);
        break;
    }
    default:
        break;
    }
    mResolvingAbilities = false;
}

void Player::makeAbilityActive(const EventPlayAbility &event) {
    mAbilityList->setActiveByUniqueId(event.unique_id(), true);
}

void Player::cancelAbility(int index) {
    doneChoosing();

    mAbilityList->activatePlay(index, false);
    mAbilityList->activateCancel(index, false);
    sendGameCommand(CommandCancelEffect());
}

void Player::abilityResolved() {
    mGame->pause(500);
    mAbilityList->removeActiveAbility();
    auto view = zone("view");
    if (view->model().count())
        QMetaObject::invokeMethod(view->visualItem(), "clear");
    auto oppView = getOpponent()->zone("view");
    if (oppView->model().count())
        QMetaObject::invokeMethod(oppView->visualItem(), "clear");

    if (!mOpponent) {
        int playableCount = 0;
        for (int i = 0; i < mAbilityList->count(); ++i) {
            const auto &abInStandby = mAbilityList->ability(i);
            if (canPlay(correspondingCard(abInStandby), abInStandby.ability))
                ++playableCount;
        }
        if (playableCount > 1) {
            // show 'Play' button on playable abilities
            for (int i = 0; i < mAbilityList->count(); ++i) {
                const auto &abInStandby = mAbilityList->ability(i);
                if (canPlay(correspondingCard(abInStandby), abInStandby.ability))
                    mAbilityList->activatePlay(i, true);
            }
        }
    }
}

void Player::activateAbilities(const EventAbilityActivated &event) {
    // this function adds activated abilities to the list
    // there's another event to make an ability active
    if (!mAbilityList->count())
        mGame->player()->stopUiInteractions();

    for (int i = 0; i < event.abilities_size(); ++i) {
        auto protoa = event.abilities(i);
        auto zoneptr = zone(protoa.zone());
        if (!zoneptr)
            return;

        bool nocard = false;
        auto card = zoneptr->findCardById(protoa.card_id());
        if (!card)
            nocard = true;

        ActivatedAbility a;
        a.uniqueId = protoa.unique_id();
        a.type = protoa.type();
        a.zone = protoa.zone();
        a.cardId = protoa.card_id();
        a.abilityId = protoa.ability_id();
        if (nocard) {
            auto cardInfo = CardDatabase::get().getCard(protoa.card_code());
            if (!cardInfo)
                return;

            a.code = cardInfo->code();
            if (protoa.type() == ProtoAbilityType::ProtoCard) {
                // if a card with a temp ability left the stage after the ability was activated
                if (static_cast<size_t>(a.abilityId) >= cardInfo->abilities().size()) {
                    if (protoa.ability().empty())
                        return;
                    a.ability = decodeAbility(protoa.ability());
                } else {
                    const auto &abuf = cardInfo->abilities()[a.abilityId];
                    a.ability = decodeAbility(abuf);
                }
                a.text = QString::fromStdString(printAbility(a.ability));
            }
        } else {
            a.code = protoa.card_code();
            if (protoa.type() == ProtoAbilityType::ProtoCard) {
                a.ability = card->abilityById(a.abilityId);
                a.text = card->textById(a.abilityId);
            }
        }
        if (protoa.type() == ProtoAbilityType::ProtoClimaxTrigger) {
            a.ability = triggerAbility(static_cast<TriggerIcon>(protoa.ability_id()));
            a.text = QString::fromStdString(printAbility(a.ability));
        } else if (protoa.type() == ProtoAbilityType::ProtoGlobal) {
            a.ability = globalAbility(static_cast<GlobalAbility>(protoa.ability_id()));
            a.text = QString::fromStdString(printAbility(a.ability));
        } else if (protoa.type() == ProtoAbilityType::ProtoDelayed) {
            // this ability is different from what the card says
            if (protoa.ability().empty())
                return;
            a.ability = decodeAbility(protoa.ability());
            a.text = QString::fromStdString(printAbility(a.ability));
        }
        a.active = false;
        mAbilityList->addAbility(a);
    }

    if (!mOpponent) {
        int playableCount = 0;
        for (int i = 0; i < mAbilityList->count(); ++i) {
            const auto &abInStandby = mAbilityList->ability(i);
            if (canPlay(correspondingCard(abInStandby), abInStandby.ability))
                ++playableCount;
        }
        if (playableCount > 1) {
            // show 'Play' button on playable abilities
            for (int i = 0; i < mAbilityList->count(); ++i) {
                const auto &abInStandby = mAbilityList->ability(i);
                if (canPlay(correspondingCard(abInStandby), abInStandby.ability))
                    mAbilityList->activatePlay(i, true);
            }
        }
    }
    if (mAbilityList->count())
        mGame->pause(450);
}

void Player::startResolvingAbility(const EventStartResolvingAbility &event) {
    for (int i = 0; i < mAbilityList->count(); ++i) {
        if (mAbilityList->ability(i).uniqueId == event.unique_id()) {
            mAbilityList->setActive(i, true);
            break;
        }
    }
}

void Player::endResolvingAbilties() {
    mAbilityList->clear();
    mGame->player()->restoreUiState();
}
