#include "player.h"

#include "abilities.h"
#include "abilityUtils.h"
#include "cardDatabase.h"
#include "codecs/decode.h"
#include "codecs/print.h"
#include "game.h"
#include "globalAbilities/globalAbilities.h"
#include "hand.h"
#include "stage.h"

namespace {
asn::ChooseCard decodeChooseCard(const std::string &buf) {
    std::vector<uint8_t> binbuf(buf.begin(), buf.end());
    auto it = binbuf.begin();
    return ::decodeChooseCard(it, binbuf.end());
}
asn::MoveCard decodeMoveCard(const std::string &buf) {
    std::vector<uint8_t> binbuf(buf.begin(), buf.end());
    auto it = binbuf.begin();
    return ::decodeMoveCard(it, binbuf.end());
}
asn::DrawCard decodeDrawCard(const std::string &buf) {
    std::vector<uint8_t> binbuf(buf.begin(), buf.end());
    auto it = binbuf.begin();
    return ::decodeDrawCard(it, binbuf.end());
}
asn::SearchCard decodeSearchCard(const std::string &buf) {
    std::vector<uint8_t> binbuf(buf.begin(), buf.end());
    auto it = binbuf.begin();
    return ::decodeSearchCard(it, binbuf.end());
}
asn::AbilityGain decodeAbilityGain(const std::string &buf) {
    std::vector<uint8_t> binbuf(buf.begin(), buf.end());
    auto it = binbuf.begin();
    return ::decodeAbilityGain(it, binbuf.end());
}
asn::Ability decodeAbility(const std::string &buf) {
    std::vector<uint8_t> binbuf(buf.begin(), buf.end());
    auto it = binbuf.begin();
    return ::decodeAbility(it, binbuf.end());
}
asn::Look decodeLook(const std::string &buf) {
    std::vector<uint8_t> binbuf(buf.begin(), buf.end());
    auto it = binbuf.begin();
    return ::decodeLook(it, binbuf.end());
}

void highlightAllCards(CardZone *zone, bool highlight) {
    for (int i = 0; i < zone->model().count(); ++i)
        zone->model().setGlow(i, highlight);
}
void selectAllCards(CardZone *zone, bool select) {
    for (int i = 0; i < zone->model().count(); ++i)
        zone->model().setSelected(i, select);
}

int highlightEligibleCards(CardZone *zone, const std::vector<asn::CardSpecifier> &specs,
                           asn::TargetMode mode, const ActivatedAbility &a) {
    int eligibleCount = 0;
    const auto &cards = zone->cards();
    highlightAllCards(zone, false);
    selectAllCards(zone, false);

    for (int i = 0; i < zone->model().count(); ++i) {
        if (!cards[i].cardPresent())
            continue;

        if ((mode == asn::TargetMode::FrontRow && i > 2) ||
            (mode == asn::TargetMode::BackRow && i < 3))
            continue;

        if (mode == asn::TargetMode::AllOther &&
            a.zone == zone->name() && a.cardId == i)
            continue;

        if (checkCard(specs, cards[i])) {
            zone->model().setGlow(i, true);
            eligibleCount++;
        }
    }

    return eligibleCount;
}
}

int Player::highlightCardsForChoice(const asn::Target &target, const asn::Place &place) {
    const auto &specs = target.targetSpecification->cards.cardSpecifiers;
    int eligibleCount = 0;
    if (place.owner == asn::Player::Player || place.owner == asn::Player::Both) {
        auto from = zone(asnZoneToString(place.zone));
        eligibleCount += highlightEligibleCards(from, specs, target.targetSpecification->mode, mAbilityList->ability(mAbilityList->activeId()));
    }
    if (place.owner == asn::Player::Opponent || place.owner == asn::Player::Both) {
        auto from = mGame->opponent()->zone(asnZoneToString(place.zone));
        eligibleCount += highlightEligibleCards(from, specs, target.targetSpecification->mode, mAbilityList->ability(mAbilityList->activeId()));
    }
    return eligibleCount;
}

void Player::processChooseCardInternal(int eligibleCount, OptionalPlace place, bool mandatory) {
    if (eligibleCount) {
        if (place && place->get().zone == asn::Zone::WaitingRoom &&
            place->get().owner == asn::Player::Player)
            QMetaObject::invokeMethod(zone("wr")->visualItem(), "openView", Q_ARG(QVariant, true));
        if (!mandatory)
            mAbilityList->activateCancel(mAbilityList->activeId(), true);
    } else {
        mGame->pause(500);
        sendGameCommand(CommandCancelEffect());
    }
}

void Player::sendChooseCard(const asn::ChooseCard &e) {
    CardZone *from;
    if (e.placeType == asn::PlaceType::Selection) {
        from = zone("view");
    } else {
        if (e.place->owner == asn::Player::Player)
            from = zone(asnZoneToString(e.place->zone));
        else if (e.place->owner == asn::Player::Opponent)
            from = mGame->opponent()->zone(asnZoneToString(e.place->zone));
    }

    CommandChooseCard cmd;
    auto zoneName = from->name();
    if (zoneName == "deckView" || zoneName == "view")
        zoneName = "deck";
    cmd.set_zone(zoneName);
    cmd.set_owner(e.place->owner == asn::Player::Player ? ProtoPlayer : ProtoOpponent);
    const auto &cards = from->cards();
    for (int i = 0; i < static_cast<int>(cards.size()); ++i) {
        if (cards[i].selected()) {
            if (from->name() == "view")
                cmd.add_ids(from->model().count() - 1 - i);
            else
                cmd.add_ids(i);
        }
    }
    sendGameCommand(cmd);
}

void Player::sendChooseCard(const asn::SearchCard &e) {
    auto player = e.place.owner == asn::Player::Player ? this : mGame->opponent();
    auto zoneName = asnZoneToString(e.place.zone);
    if (zoneName == "deck")
        zoneName = "deckView";
    auto from = player->zone(zoneName);

    CommandChooseCard cmd;
    cmd.set_zone(std::string(asnZoneToString(e.place.zone)));
    cmd.set_owner(e.place.owner == asn::Player::Player ? ProtoPlayer : ProtoOpponent);
    const auto &cards = from->cards();
    for (int i = 0; i < static_cast<int>(cards.size()); ++i) {
        if (cards[i].selected()) {
            cmd.add_ids(i);
        }
    }
    sendGameCommand(cmd);
}

void Player::dehighlightCards(asn::PlaceType placeType, OptionalPlace place) {
    if (placeType == asn::PlaceType::Selection) {
        auto from = zone("view");
        highlightAllCards(from, false);
        selectAllCards(from, false);
        return;
    }
    if (place->get().zone == asn::Zone::WaitingRoom) {
        QMetaObject::invokeMethod(zone("wr")->visualItem(), "openView", Q_ARG(QVariant, false));
    }
    if (place->get().owner == asn::Player::Player || place->get().owner == asn::Player::Both) {
        auto from = zone(asnZoneToString(place->get().zone));
        highlightAllCards(from, false);
        selectAllCards(from, false);
    }
    if (place->get().owner == asn::Player::Opponent || place->get().owner == asn::Player::Both) {
        auto from = mGame->opponent()->zone(asnZoneToString(place->get().zone));
        highlightAllCards(from, false);
        selectAllCards(from, false);
    }
}

void Player::highlightPlayableCards() {
    auto &cards = mHand->cards();
    for (int i = 0; i < static_cast<int>(cards.size()); ++i) {
        if (canPlay(cards[i]))
            mHand->model().setGlow(i, true);
        else
            mHand->model().setGlow(i, false);
    }
}

void Player::processChooseCard(const EventChooseCard &event) {
    if (mOpponent)
        return;

    auto effect = decodeChooseCard(event.effect());
    if (effect.targets.size() != 1 ||
        effect.targets[0].type != asn::TargetType::SpecificCards) {
        assert(false);
        return;
    }

    const auto &spec = *effect.targets[0].targetSpecification;
    bool mandatory = event.mandatory();
    if (spec.number.mod == asn::NumModifier::UpTo)
        mandatory = false;

    int eligibleCardsNum;
    bool fromView = effect.placeType == asn::PlaceType::Selection;
    if (fromView)
        eligibleCardsNum = highlightEligibleCards(zone("view"), spec.cards.cardSpecifiers,
                                                  spec.mode, mAbilityList->ability(mAbilityList->activeId()));
    else
        eligibleCardsNum = highlightCardsForChoice(effect.targets[0], *effect.place);
    if (eligibleCardsNum)
        mAbilityList->ability(mAbilityList->activeId()).effect = effect;
    processChooseCardInternal(eligibleCardsNum, fromView ? std::nullopt : OptionalPlace(*effect.place), mandatory);
}

void Player::processSearchCard(const EventSearchCard &event) {
    auto effect = decodeSearchCard(event.effect());
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

    auto effect = decodeMoveCard(event.effect());
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

    auto effect = decodeMoveCard(event.effect());
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
    auto effect = decodeMoveCard(event.effect());
    if ((mOpponent && effect.executor == asn::Player::Player) ||
        (!mOpponent && effect.executor == asn::Player::Opponent))
        return;

    assert(effect.to.size() == 1);
    auto &a = mAbilityList->ability(mAbilityList->activeId());
    a.effect = effect;
    a.choiceType = ChoiceType::DestinationIndex;
    if (effect.to[0].pos == asn::Position::EmptySlotBackRow) {
        mStage->model().setGlow(3, true);
        mStage->model().setGlow(4, true);
    } else if (effect.to[0].zone == asn::Zone::Stage && effect.to[0].pos == asn::Position::NotSpecified) {
        auto player = effect.executor == asn::Player::Opponent ? getOpponent() : this;
        highlightAllCards(player->zone("stage"), true);
    }
}

void Player::processMoveTargetChoice(const EventMoveTargetChoice &event) {
    if (mOpponent)
        return;

    auto effect = decodeMoveCard(event.effect());
    assert(effect.executor == asn::Player::Player);
    assert(effect.target.type == asn::TargetType::SpecificCards);
    assert(effect.from.pos == asn::Position::NotSpecified);

    auto effectText = printMoveCard(effect);
    effectText[0] = std::toupper(effectText[0]);
    mGame->showText(QString::fromStdString(effectText));
    int eligibleCount = highlightCardsForChoice(effect.target, effect.from);
    if (eligibleCount)
        mAbilityList->ability(mAbilityList->activeId()).effect = effect;
    processChooseCardInternal(eligibleCount, effect.from, event.mandatory());
}

void Player::processDrawChoice(const EventDrawChoice &event) {
    if (mOpponent)
        return;

    auto effect = decodeDrawCard(event.effect());

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

    auto effect = decodeAbilityGain(event.effect());
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

void Player::processAbilityGain(const EventAbilityGain &event) {
    auto ability = decodeAbility(event.ability());
    auto pzone = zone(event.zone());
    auto &card = pzone->cards()[event.cardid()];
    if (!card.cardPresent())
        return;
    card.addAbility(ability);
}

void Player::processRemoveAbility(const EventRemoveAbility &event) {
    auto pzone = zone(event.zone());
    auto &card = pzone->cards()[event.cardid()];
    if (!card.cardPresent())
        return;
    card.removeAbility(event.abilityid());
}

void Player::processLook(const EventLook &event) {
    if (mOpponent)
        return;

    auto effect = decodeLook(event.effect());
    if (effect.number.mod != asn::NumModifier::UpTo)
        return;

    auto &activatedAbility = mAbilityList->ability(mAbilityList->activeId());
    activatedAbility.effect = effect;
    if (static_cast<asn::EffectType>(event.nexteffecttype()) == asn::EffectType::MoveCard) {
        activatedAbility.nextEffect = decodeMoveCard(event.nexteffect());
        auto &moveEffect = std::get<asn::MoveCard>(activatedAbility.nextEffect);
        if (moveEffect.order == asn::Order::Any)
            zone("view")->visualItem()->setProperty("mDragEnabled", true);
    } else if (static_cast<asn::EffectType>(event.nexteffecttype()) == asn::EffectType::ChooseCard)
        mAbilityList->ability(mAbilityList->activeId()).nextEffect = decodeChooseCard(event.nexteffect());

    zone("deck")->visualItem()->setProperty("mGlow", true);
    mAbilityList->activateCancel(mAbilityList->activeId(), true);
}

void Player::revealTopDeck(const EventRevealTopDeck &event) {
    zone("view")->visualItem()->setProperty("mViewMode", Game::RevealMode);

    QString code = QString::fromStdString(event.code());
    mGame->pause(600);
    createMovingCard(code, "deck", 0, "view", 0, false, true, true);
}

void Player::lookTopDeck(const EventLookTopDeck &event) {
    auto view = zone("view");
    view->visualItem()->setProperty("mViewMode", Game::LookMode);

    QString code = QString::fromStdString(event.code());
    mGame->pause(400);
    createMovingCard(code, "deck", 0, "view", 0, false, true, true);

    if (!mAbilityList->count())
        return;

    auto &activeAbility = mAbilityList->ability(mAbilityList->activeId());
    auto &effect = activeAbility.effect;
    if (std::holds_alternative<asn::Look>(effect)) {
        const auto &look = std::get<asn::Look>(effect);
        if (look.number.mod == asn::NumModifier::UpTo) {
            if (view->model().count() + 1 < look.number.value)
                zone("deck")->visualItem()->setProperty("mGlow", true);
            else
                zone("deck")->visualItem()->setProperty("mGlow", false);
        }
        if (std::holds_alternative<asn::MoveCard>(activeAbility.nextEffect)) {
            mAbilityList->activatePlay(mAbilityList->activeId(), true, "Submit");
        } else if (std::holds_alternative<asn::ChooseCard>(activeAbility.nextEffect)) {
            const auto &chooseEffect = std::get<asn::ChooseCard>(activeAbility.nextEffect);
            if (chooseEffect.targets[0].type == asn::TargetType::SpecificCards) {
                const auto &spec = *chooseEffect.targets[0].targetSpecification;
                if (spec.number.mod == asn::NumModifier::UpTo)
                    mAbilityList->activateCancel(mAbilityList->activeId(), true);
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

        const auto &cards = zoneptr->cards();
        if (protoa.cardid() < 0)
            return;

        bool nocard = false;
        if (static_cast<size_t>(protoa.cardid()) >= cards.size() ||
            cards[protoa.cardid()].code() != protoa.cardcode())
            nocard = true;

        asn::Ability ability;
        ActivatedAbility a;
        a.uniqueId = protoa.uniqueid();
        a.type = protoa.type();
        a.zone = protoa.zone();
        a.cardId = protoa.cardid();
        a.abilityId = protoa.abilityid();
        if (nocard) {
            auto cardInfo = CardDatabase::get().getCard(protoa.cardcode());
            if (!cardInfo)
                return;

            a.code = cardInfo->code();
            if (protoa.type() == ProtoAbilityType::ProtoCard) {
                const auto &abuf = cardInfo->abilities()[a.abilityId];
                a.ability = decodeAbility(abuf);
                a.text = QString::fromStdString(printAbility(ability));
            }
        } else {
            a.code = protoa.cardcode();
            if (protoa.type() == ProtoAbilityType::ProtoCard) {
                const auto &card = zone(a.zone)->cards()[a.cardId];
                a.ability = card.ability(a.abilityId);
                a.text = card.text(a.abilityId);
            }
        }
        if (protoa.type() == ProtoAbilityType::ProtoClimaxTrigger) {
            a.ability = triggerAbility(static_cast<TriggerIcon>(protoa.abilityid()));
            a.text = QString::fromStdString(printAbility(a.ability));
        } else if (protoa.type() == ProtoAbilityType::ProtoGlobal) {
            a.ability = globalAbility(static_cast<GlobalAbility>(protoa.abilityid()));
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
        if (mAbilityList->ability(i).uniqueId == event.uniqueid()) {
            mAbilityList->setActive(i, true);
            break;
        }
    }
}

void Player::endResolvingAbilties() {
    mAbilityList->clear();
    mGame->player()->restoreUiState();
}

void Player::playAbility(int index) {
    const auto &ab = mAbilityList->ability(index);
    if (ab.active) {
        mAbilityList->activatePlay(index, false);
        mAbilityList->activateCancel(index, false);
        if (std::holds_alternative<asn::Look>(ab.effect)) {
            if (std::holds_alternative<asn::MoveCard>(ab.nextEffect)) {
                const auto &moveEffect = std::get<asn::MoveCard>(ab.nextEffect);
                if (moveEffect.order == asn::Order::Any) {
                    auto view = zone("view");
                    view->visualItem()->setProperty("mDragEnabled", false);
                    QVariant codesv;
                    QMetaObject::invokeMethod(view->visualItem(), "getCardOrder", Q_RETURN_ARG(QVariant, codesv));
                    CommandMoveInOrder cmd;
                    auto codes = codesv.toStringList();
                    for (int i = 0; i < codes.size(); ++i)
                        cmd.add_codes(codes[i].toStdString());
                    sendGameCommand(cmd);
                    QMetaObject::invokeMethod(view->visualItem(), "clear");
                    doneChoosing();
                }
            } else if (std::holds_alternative<asn::ChooseCard>(ab.nextEffect)) {
                const auto &chooseEffect = std::get<asn::ChooseCard>(ab.effect);
                sendChooseCard(chooseEffect);
                doneChoosing();
            }
        } else if (std::holds_alternative<asn::ChooseCard>(ab.effect)) {
            const auto &chooseEffect = std::get<asn::ChooseCard>(ab.effect);
            sendChooseCard(chooseEffect);
            doneChoosing();
        } else if (std::holds_alternative<asn::SearchCard>(ab.effect)) {
            const auto &searchEffect = std::get<asn::SearchCard>(ab.effect);
            sendChooseCard(searchEffect);
            doneChoosing();
        } else {
            sendGameCommand(CommandPlayEffect());
        }
    } else {
        mAbilityList->setActive(index, true);
        for (int i = 0; i < mAbilityList->count(); ++i) {
            mAbilityList->activatePlay(i, false);
        }
        CommandPlayAbility cmd;
        cmd.set_uniqueid(ab.uniqueId);
        sendGameCommand(cmd);
    }
}

void Player::doneChoosing() {
    auto &a = mAbilityList->ability(mAbilityList->activeId());
    auto &effect = mAbilityList->ability(mAbilityList->activeId()).effect;
    if (std::holds_alternative<asn::ChooseCard>(effect)) {
        auto &ef = std::get<asn::ChooseCard>(effect);
        dehighlightCards(ef.placeType, ef.place ? OptionalPlace(*ef.place) : std::nullopt);
    } else if (std::holds_alternative<asn::MoveCard>(effect)) {
        auto &ef = std::get<asn::MoveCard>(effect);
        dehighlightCards(asn::PlaceType::SpecificPlace, ef.from);
        dehighlightCards(asn::PlaceType::SpecificPlace, ef.to[0]);
    } else if (std::holds_alternative<asn::SearchCard>(effect)) {
        mDeckView->hide();
    } else if (std::holds_alternative<asn::Look>(effect)) {
        zone("deck")->visualItem()->setProperty("mGlow", false);
        if (std::holds_alternative<asn::ChooseCard>(a.nextEffect)) {
            auto &ef = std::get<asn::ChooseCard>(a.nextEffect);
            dehighlightCards(ef.placeType, ef.place ? OptionalPlace(*ef.place) : std::nullopt);
        }
    }
    mGame->hideText();
}

void Player::abilityResolved() {
    mGame->pause(500);
    mAbilityList->removeActiveAbility();
    auto view = zone("view");
    if (view->model().count())
        QMetaObject::invokeMethod(view->visualItem(), "clear");

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
        const auto &card = activePlayer->mStage->cards()[activePlayer->mAttackingId];
        if (card.cardPresent() && card.state() == StateRested)
            activePlayer->mStage->model().setSelected(activePlayer->mAttackingId, true);
        break;
    }
    default:
        break;
    }
}

void Player::makeAbilityActive(const EventPlayAbility &event) {
    mAbilityList->setActiveByUniqueId(event.uniqueid(), true);
}

void Player::conditionNotMet() {
    if (zone("view")->model().count())
        mGame->pause(900);
}

void Player::payCostChoice() {
    if (mOpponent)
        return;

    mAbilityList->activatePlay(mAbilityList->activeId(), true, "Pay cost");
    mAbilityList->activateCancel(mAbilityList->activeId(), true);
}

void Player::setCannotPlay(const EventSetCannotPlay &event) {
    if (event.handid() >= mHand->model().count())
        return;

    mHand->cards()[event.handid()].setCannotPlay(event.cannotplay());
    if (mHand->isPlayTiming())
        highlightPlayableCards();
}

const Card &Player::correspondingCard(const ActivatedAbility &abilityDescriptor) {
    static Card dummyCard(zone("deck"));
    auto pzone = zone(abilityDescriptor.zone);
    if (pzone->model().count() <= abilityDescriptor.cardId)
        return dummyCard;
    const auto &card = pzone->cards()[abilityDescriptor.cardId];
    if (card.code() != abilityDescriptor.code)
        return dummyCard;
    // we miss the situation when the card is replaced with card with the same code
    return card;
}

void Player::cancelAbility(int index) {
    doneChoosing();

    mAbilityList->activatePlay(index, false);
    mAbilityList->activateCancel(index, false);
    sendGameCommand(CommandCancelEffect());
}

void Player::chooseCardOrPosition(int index, QString qzone, bool opponent) {
    if (!mAbilityList->count()) {
        if (mOpponent)
            return;
        getOpponent()->chooseCardOrPosition(index, qzone, opponent);
        return;
    }

    int activeId = mAbilityList->activeId();
    if (mAbilityList->ability(activeId).choiceType == ChoiceType::Card)
        chooseCard(index, qzone, opponent);
    else
        sendChoice(index);
}

void Player::chooseCard(int, QString qzone, bool opponent) {
    CardZone *pzone;
    if (!opponent)
        pzone = zone(qzone.toStdString());
    else
        pzone = mGame->opponent()->zone(qzone.toStdString());

    // someday we'll need to count selected and highlighted in all needed zones
    int selected = pzone->numOfSelectedCards();
    int highlighted = pzone->numOfHighlightedCards();

    int activeId = mAbilityList->activeId();
    auto &effect = mAbilityList->ability(activeId).effect;
    const asn::Number *number;
    if (std::holds_alternative<asn::ChooseCard>(effect) || std::holds_alternative<asn::Look>(effect)) {
        const auto &ef = std::holds_alternative<asn::ChooseCard>(effect) ?
                    std::get<asn::ChooseCard>(effect) :
                    std::get<asn::ChooseCard>(mAbilityList->ability(activeId).nextEffect);
        assert(ef.targets.size() == 1);
        if (ef.targets[0].type != asn::TargetType::SpecificCards)
            return;

        number = &ef.targets[0].targetSpecification->number;
    } else if (std::holds_alternative<asn::SearchCard>(effect)) {
        auto &ef = std::get<asn::SearchCard>(effect);
        assert(ef.targets.size() == 1);
        number = &ef.targets[0].number;
    } else if (std::holds_alternative<asn::MoveCard>(effect)) {
        auto &ef = std::get<asn::MoveCard>(effect);
        if (ef.target.type != asn::TargetType::SpecificCards)
            return;

        number = &ef.target.targetSpecification->number;
    }

    if (number->value != selected && selected < highlighted) {
        if (number->mod == asn::NumModifier::UpTo) {
            mAbilityList->activatePlay(activeId, selected != 0, "Choose");
        }
        return;
    }

    mAbilityList->activatePlay(activeId, false);
    mAbilityList->activateCancel(activeId, false);

    CommandChooseCard cmd;
    std::string cmdZone = qzone.toStdString();
    if (cmdZone == "deckView" || cmdZone == "view")
        cmdZone = "deck";
    cmd.set_zone(cmdZone);
    cmd.set_owner(opponent ? ProtoOpponent : ProtoPlayer);
    const auto &cards = pzone->cards();
    for (int i = 0; i < static_cast<int>(cards.size()); ++i) {
        if (cards[i].selected()) {
            if (qzone == "view")
                cmd.add_ids(zone("deck")->model().count() - 1 - i);
            else
                cmd.add_ids(i);
        }
    }
    sendGameCommand(cmd);
    doneChoosing();
}

void Player::sendChoice(int index) {
    int activeId = mAbilityList->activeId();
    auto &effect = mAbilityList->ability(activeId).effect;
    if (std::holds_alternative<asn::AbilityGain>(effect)) {
        auto &ef = std::get<asn::AbilityGain>(effect);
        if (static_cast<size_t>(index) > ef.abilities.size())
            return;
        if (ef.target.type == asn::TargetType::ThisCard) {
            auto pzone = zone(mAbilityList->ability(activeId).zone);
            auto &card = pzone->cards()[mAbilityList->ability(activeId).cardId];
            if (!card.cardPresent())
                return;
        }
    }
    CommandChoice cmd;
    cmd.set_choice(index);
    sendGameCommand(cmd);
    doneChoosing();
}

void Player::playActAbility(int index) {
    if (mGame->phase() != asn::Phase::MainPhase || mAbilityList->count())
        return;

    const auto &card = mStage->cards()[index];
    if (!card.cardPresent())
        return;

    std::unique_ptr<ActChoiceDialog> choiceDlg;
    for (int i = 0; i < card.abilityCount(); ++i){
        const auto &ab = card.ability(i);
        if (ab.type == asn::AbilityType::Act) {
            if (canPlay(card, ab)) {
                if (!choiceDlg)
                    choiceDlg = std::make_unique<ActChoiceDialog>(mGame);
                choiceDlg->addAbility(index, i, card.text(i));
            }
        }
    }

    if (choiceDlg) {
        choiceDlg->show();
        mChoiceDialog = std::move(choiceDlg);
    }
}

bool Player::canPay(const Card &thisCard, const asn::CostItem &c) const {
    if (c.type == asn::CostType::Stock) {
        const auto &item = std::get<asn::StockCost>(c.costItem);
        if (item.value > zone("stock")->model().count())
            return false;
        return true;
    } else {
        const auto &item = std::get<asn::Effect>(c.costItem);
        if (item.type == asn::EffectType::MoveCard) {
            const auto &e = std::get<asn::MoveCard>(item.effect);
            if (e.from.zone == asn::Zone::Hand &&
                e.target.targetSpecification->number.value > zone("hand")->model().count())
                return false;
        } else if (item.type == asn::EffectType::ChangeState) {
            const auto &e = std::get<asn::ChangeState>(item.effect);
            if (e.state == protoStateToState(thisCard.state()))
                return false;
        }
        return true;
    }
}

bool Player::canPlay(const Card &thisCard, const asn::Ability &a) const {
    if (a.type == asn::AbilityType::Auto) {
        const auto &aa = std::get<asn::AutoAbility>(a.ability);
        if (!aa.cost)
            return true;
        for (const auto &costItem: aa.cost->items)
            if (!canPay(thisCard, costItem))
                return false;
        return true;
    } else if (a.type == asn::AbilityType::Act) {
        const auto &aa = std::get<asn::ActAbility>(a.ability);
        for (const auto &costItem: aa.cost.items)
            if (!canPay(thisCard, costItem))
                return false;
    }

    return true;
}

bool Player::canPlayCounter(const Card &card) const {
    if (!card.isCounter())
        return false;
    if (card.type() == CardType::Event)
        return true;
    for (int i = 0; i < card.abilityCount(); ++i) {
        const auto &a = card.ability(i);
        if (a.type != asn::AbilityType::Act)
            continue;
        const auto &aa = std::get<asn::ActAbility>(a.ability);
        if (aa.keywords.empty() || aa.keywords[0] != asn::Keyword::Backup)
            continue;
        if (canPlay(card, a))
            return true;
    }

    return false;
}

void Player::lookOrRevealTopDeck() {
    if (!mAbilityList->count())
        return;

    auto effect = mAbilityList->ability(mAbilityList->activeId()).effect;
    if (std::holds_alternative<asn::Look>(effect)) {
        zone("deck")->visualItem()->setProperty("mGlow", false);

        // deactivate buttons until a new card is revealed
        mAbilityList->activateCancel(mAbilityList->activeId(), false);
        mAbilityList->activatePlay(mAbilityList->activeId(), false);
        sendGameCommand(CommandLookTopDeck());
    }
}

void Player::cardInserted(QString targetZone) {
    if (mOpponent)
        return;

    if (targetZone == "view") {
        if (!mAbilityList->count())
            return;

        auto &a = mAbilityList->ability(mAbilityList->activeId());
        if (std::holds_alternative<asn::Look>(a.effect)) {
            if (std::holds_alternative<asn::ChooseCard>(a.nextEffect)) {
                const auto &chooseEffect = std::get<asn::ChooseCard>(a.nextEffect);
                if (chooseEffect.targets[0].type == asn::TargetType::SpecificCards) {
                    const auto &spec = *chooseEffect.targets[0].targetSpecification;
                    if (chooseEffect.placeType == asn::PlaceType::Selection) {
                        auto from = zone("view");
                        highlightEligibleCards(from, spec.cards.cardSpecifiers, spec.mode, a);
                    }
                }
            }
        }
    }
}
