#include "player.h"

#include "abilityCommands.pb.h"
#include "abilityEvents.pb.h"

#include "abilities.h"
#include "abilityUtils.h"
#include "codecs/decode.h"
#include "game.h"
#include "globalAbilities/globalAbilities.h"
#include "hand.h"
#include "stage.h"
#include "utils.h"

namespace {
template<typename F>
auto decodingWrapper(const std::string &buf, F &decodeFunction) {
    std::vector<uint8_t> binbuf(buf.begin(), buf.end());
    auto it = binbuf.cbegin();
    return decodeFunction(it, binbuf.end());
}
}

int Player::highlightCardsFromEvent(
        const EventChooseCard &event,
        const asn::ChooseCard &effect) {
    bool fromView = effect.targets[0].placeType == asn::PlaceType::Selection;
    std::map<std::string, CardZone*> zones;
    bool considerCannotBeChosen = false;
    if (fromView) {
        zones.emplace("view", getViewWithCards());
    } else {
        for (const auto &target: effect.targets) {
            const auto &place = *target.place;
            auto player = place.owner == asn::Player::Player ? this : getOpponent();
            zones.emplace(asnZoneToString(place.zone), player->zone(place.zone));
            // for now let's consider that's impossible to choose from both players
            considerCannotBeChosen = (place.owner == asn::Player::Opponent) ==
                                     (effect.executor == asn::Player::Player) &&
                    event.consider_cannot_be_chosen();
        }
    }

    for (auto &zone: zones) {
        dehighlightAllCards(zone.second);
        deselectAllCards(zone.second);
    }

    int eligibleCount = 0;

    auto highlightCard =
            [considerCannotBeChosen, &eligibleCount] (CardZone *zone, int pos) {
        const auto &cards = zone->cards();
        if (!cards[pos].cardPresent())
            return;

        if (considerCannotBeChosen && cards[pos].cannotBeChosen())
            return;

        zone->model().setGlow(pos, true);
        eligibleCount++;
    };

    for (int i = 0; i < event.cards_size(); ++i) {
        auto &event_card = event.cards(i);
        if (fromView) {
            auto view = zones["view"];
            const auto &cards = view->cards();
            for (size_t i = 0; i < cards.size(); ++i) {
                if (cards[i].id() == event_card.id()) {
                    highlightCard(view, static_cast<int>(i));
                    break;
                }
            }
        } else {
            if (!zones.contains(event_card.zone())) {
                qWarning() << "met unmatched zone in EventChooseCard";
                continue;
            }
            highlightCard(zones[event_card.zone()], event_card.position());
        }
    }

    return eligibleCount;
}

int Player::highlightCardsForChoice(const asn::Target &target, const asn::Place &place,
                                    const std::optional<asn::ChooseCard> &chooseEffect) {
    const auto &specs = target.targetSpecification->cards.cardSpecifiers;
    int eligibleCount = 0;
    bool considerCannotBeChosen = false;
    if (chooseEffect)
        considerCannotBeChosen = (place.owner == asn::Player::Opponent) ==
                                 (chooseEffect->executor == asn::Player::Player);
    if (place.owner == asn::Player::Player || place.owner == asn::Player::Both) {
        auto from = zone(place.zone);
        eligibleCount += highlightEligibleCards(from, specs, target.targetSpecification->mode,
                                                mAbilityList->ability(mAbilityList->activeId()),
                                                considerCannotBeChosen);
    }
    if (place.owner == asn::Player::Opponent || place.owner == asn::Player::Both) {
        auto from = getOpponent()->zone(place.zone);
        eligibleCount += highlightEligibleCards(from, specs, target.targetSpecification->mode,
                                                mAbilityList->ability(mAbilityList->activeId()),
                                                considerCannotBeChosen);
    }
    return eligibleCount;
}

void Player::toggleZoneView(const asn::Place &place, bool open) {
    if (place.zone != asn::Zone::WaitingRoom && place.zone != asn::Zone::Memory)
        return;
    Player *player = place.owner == asn::Player::Player ? this : getOpponent();
    QMetaObject::invokeMethod(player->zone(asnZoneToString(place.zone))->visualItem(),
                              "openView", Q_ARG(QVariant, open));
}

void Player::processChooseCardInternal(int eligibleCount, const std::vector<asn::Place> &places, bool mandatory,
                                       asn::Player executor) {
    if (eligibleCount) {
        for (const auto &place: places) {
            toggleZoneView(place, true);
        }
        if (!mandatory)
            mAbilityList->activateCancel(mAbilityList->activeId(), true);
    } else {
        mGame->pause(600);
        if (executor == asn::Player::Player)
            sendGameCommand(CommandCancelEffect());
        else
            getOpponent()->sendGameCommand(CommandCancelEffect());
    }
}

void Player::sendChooseCard(const asn::ChooseCard &e) {
    // choosing from multiple zones is not supported
    CardZone *from;
    if (e.targets[0].placeType == asn::PlaceType::Selection) {
        from = getViewWithCards();
    } else {
        if (e.targets[0].place->owner == asn::Player::Player)
            from = zone(e.targets[0].place->zone);
        else if (e.targets[0].place->owner == asn::Player::Opponent)
            from = getOpponent()->zone(e.targets[0].place->zone);
        else
            throw std::runtime_error("shouldn't happen");
    }

    CommandChooseCard cmd;
    auto zoneName = from->name();
    if (zoneName == "deckView" || zoneName == "view")
        zoneName = "deck";
    cmd.set_zone(zoneName);
    cmd.set_owner(from->player() == this ? ProtoPlayer : ProtoOpponent);

    auto actualZone = from->player()->zone(zoneName);
    const auto &cards = from->cards();
    for (int i = 0; i < static_cast<int>(cards.size()); ++i) {
        if (cards[i].selected()) {
            if (from->name() == "view")
                cmd.add_positions(actualZone->model().count() - 1 - i);
            else
                cmd.add_positions(i);
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
            cmd.add_positions(i);
        }
    }
    sendGameCommand(cmd);
}

void Player::dehighlightCards(const std::vector<asn::TargetAndPlace> &targets) {
    for (const auto &target: targets) {
        dehighlightCards(target.placeType, target.place);
    }
}

void Player::dehighlightCards(asn::PlaceType placeType, std::optional<asn::Place> place) {
    if (placeType == asn::PlaceType::Selection) {
        auto from = getViewWithCards();
        dehighlightAllCards(from);
        deselectAllCards(from);
        return;
    }
    if (placeType != asn::PlaceType::Selection  ||
            !place.has_value()) {
        return;
    }

    toggleZoneView(place.value(), false);
    if (place.value().zone == asn::Zone::NotSpecified)
        return;
    if (place.value().owner == asn::Player::Player || place.value().owner == asn::Player::Both) {
        auto from = zone(place.value().zone);
        dehighlightAllCards(from);
        deselectAllCards(from);
    }
    if (place.value().owner == asn::Player::Opponent || place.value().owner == asn::Player::Both) {
        auto from = getOpponent()->zone(place.value().zone);
        dehighlightAllCards(from);
        deselectAllCards(from);
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

void Player::playAbility(int index) {
    const auto &ab = mAbilityList->ability(index);
    if (ab.active) {
        mAbilityList->activatePlay(index, false);
        mAbilityList->activateCancel(index, false);
        if (ab.awaitingConfirmation) {
            sendGameCommand(CommandPlayEffect());
            return;
        }
        if (std::holds_alternative<asn::Look>(ab.effect) ||
            std::holds_alternative<asn::RevealCard>(ab.effect)) {
            if (std::holds_alternative<asn::MoveCard>(ab.nextEffect)) {
                const auto &moveEffect = std::get<asn::MoveCard>(ab.nextEffect);
                if (moveEffect.order == asn::Order::Any) {
                    auto view = getViewWithCards();
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
            } else if (std::holds_alternative<std::monostate>(ab.nextEffect)) {
                sendGameCommand(CommandPlayEffect());
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
        cmd.set_unique_id(ab.uniqueId);
        sendGameCommand(cmd);
    }
}

void Player::doneChoosing() {
    if (!hasActivatedAbilities()) {
        if (mOpponent)
            return;

        getOpponent()->doneChoosing();
        return;
    }
    auto &a = activeAbility();
    auto &effect = a.effect;
    if (std::holds_alternative<asn::ChooseCard>(effect)) {
        auto &ef = std::get<asn::ChooseCard>(effect);
        dehighlightCards(ef.targets);
    } else if (std::holds_alternative<asn::MoveCard>(effect)) {
        auto &ef = std::get<asn::MoveCard>(effect);
        dehighlightCards(asn::PlaceType::SpecificPlace, ef.from);
        dehighlightCards(asn::PlaceType::SpecificPlace, ef.to[0]);
    } else if (std::holds_alternative<asn::SearchCard>(effect)) {
        mDeckView->hide();
    } else if (std::holds_alternative<asn::Look>(effect) ||
               std::holds_alternative<asn::RevealCard>(effect)) {
        zone("deck")->visualItem()->setProperty("mGlow", false);
        getOpponent()->zone("deck")->visualItem()->setProperty("mGlow", false);
        if (std::holds_alternative<asn::ChooseCard>(a.nextEffect)) {
            auto &ef = std::get<asn::ChooseCard>(a.nextEffect);
            dehighlightCards(ef.targets);
        }
    } else if (std::holds_alternative<asn::ChangeState>(effect)) {
        asn::Place place{ asn::Position::NotSpecified, asn::Zone::Stage, asn::Player::Both };
        dehighlightCards(asn::PlaceType::SpecificPlace, place);
    }
    mGame->hideText();
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

const Card &Player::correspondingCard(const ActivatedAbility &abilityDescriptor) {
    static Card dummyCard(zone("deck"));
    auto pzone = zone(abilityDescriptor.zone);
    if (auto card = pzone->findCardById(abilityDescriptor.cardId))
        return *card;
    return dummyCard;
}

void Player::chooseCardOrPosition(int index, QString qzone, bool opponent) {
    if (!hasActivatedAbilities() && !getOpponent()->hasActivatedAbilities())
        return;

    ActivatedAbility *a;
    if (hasActivatedAbilities())
        a = &activeAbility();
    else
        a = &getOpponent()->activeAbility();
    if (a->choiceType == ChoiceType::Card)
        chooseCard(index, qzone, opponent);
    else
        sendChoice(index);
}

void Player::chooseCard(int, QString qzone, bool opponent) {
    auto pzone = getOpponent(opponent)->zone(qzone.toStdString());

    int selected = pzone->numOfSelectedCards();
    int highlighted = pzone->numOfHighlightedCards();

    ActivatedAbility *a;
    if (hasActivatedAbilities())
        a = &activeAbility();
    else
        a = &getOpponent()->activeAbility();
    auto &effect = a->effect;
    asn::Number number;
    if (std::holds_alternative<asn::ChooseCard>(effect) || std::holds_alternative<asn::Look>(effect)
            || std::holds_alternative<asn::RevealCard>(effect)) {
        const auto &ef = std::holds_alternative<asn::ChooseCard>(effect) ?
                    std::get<asn::ChooseCard>(effect) :
                    std::get<asn::ChooseCard>(a->nextEffect);
        assert(ef.targets.size() > 0);
        if (ef.targets[0].target.type != asn::TargetType::SpecificCards)
            return;

        number = ef.targets[0].target.targetSpecification->number;
    } else if (std::holds_alternative<asn::SearchCard>(effect)) {
        auto &ef = std::get<asn::SearchCard>(effect);
        assert(ef.targets.size() == 1);
        number = ef.targets[0].number;
    } else if (std::holds_alternative<asn::MoveCard>(effect)) {
        auto &ef = std::get<asn::MoveCard>(effect);
        if (ef.target.type != asn::TargetType::SpecificCards)
            return;

        number = ef.target.targetSpecification->number;
    } else if (std::holds_alternative<asn::ChangeState>(effect)) {
        number = { asn::NumModifier::ExactMatch, 1 };
    } else {
        assert(false);
    }

    // only works with one zone, let's pretend we won't have to choose from multiple zones
    // in one effect
    if (number.value != selected && selected < highlighted) {
        if (number.mod == asn::NumModifier::UpTo && hasActivatedAbilities()) {
            mAbilityList->activatePlay(mAbilityList->activeId(), selected != 0, "Choose");
        }
        return;
    }

    if (hasActivatedAbilities()) {
        mAbilityList->activatePlay(mAbilityList->activeId(), false);
        mAbilityList->activateCancel(mAbilityList->activeId(), false);
    }

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
                cmd.add_positions(getOpponent(opponent)->zone("deck")->model().count() - 1 - i);
            else
                cmd.add_positions(i);
        }
    }
    sendGameCommand(cmd);
    doneChoosing();
}

void Player::sendChoice(int index) {
    if (hasActivatedAbilities()) {
        auto &a = activeAbility();
        auto &effect = a.effect;
        if (std::holds_alternative<asn::AbilityGain>(effect)) {
            auto &ef = std::get<asn::AbilityGain>(effect);
            if (static_cast<size_t>(index) > ef.abilities.size())
                return;
            if (ef.target.type == asn::TargetType::ThisCard) {
                auto pzone = zone(a.zone);
                auto card = pzone->findCardById(a.cardId);
                if (!card)
                    return;
            }
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
                auto &abilityInfo = card.abilityInfo(i);
                choiceDlg->addAbility(index, abilityInfo.id, abilityInfo.text);
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

            std::vector<const Card*> targets;
            // do not check top and bottom cards
            // only their presence
            if (e.from.pos == asn::Position::Top ||
                e.from.pos == asn::Position::Bottom) {
                const auto &cards = zone(e.from.zone)->cards();
                for (int i = 0; i < cards.size(); ++i) {
                    targets.push_back(&cards[i]);
                }
            } else {
                targets = getTargets(thisCard, e.target, e.from.zone);
            }
            if (targets.empty())
                return false;

            if (e.target.type == asn::TargetType::SpecificCards) {
                const auto &spec = *e.target.targetSpecification;
                if (spec.number.value > static_cast<int>(targets.size()))
                    return false;
            }
        } else if (item.type == asn::EffectType::ChangeState) {
            const auto &e = std::get<asn::ChangeState>(item.effect);
            if (e.target.type == asn::TargetType::ThisCard) {
                if (!thisCard.cardPresent())
                    return false;
                if (e.state == thisCard.state())
                    return false;
            } else if (e.target.type == asn::TargetType::SpecificCards) {
                int num = 0;
                auto targets = getTargets(thisCard, e.target);
                for (auto target: targets) {
                    if (e.state != target->state())
                        num++;
                }
                const auto &spec = *e.target.targetSpecification;
                assert(spec.number.mod == asn::NumModifier::ExactMatch);
                if (num < spec.number.value)
                    return false;
            } else {
                assert(false);
            }
        } else if (item.type == asn::EffectType::RevealCard) {
            const auto &e = std::get<asn::RevealCard>(item.effect);
            assert(e.number.value == 1);
            auto hand = zone("hand");
            for (int i = 0; i < hand->model().count(); ++i)
                if (checkCard(e.card->cardSpecifiers, hand->model().cards()[i]))
                    return true;

            return false;
        } else if (item.type == asn::EffectType::AddMarker) {
            const auto &e = std::get<asn::AddMarker>(item.effect);
            std::vector<const Card*> targets;
            // do not check top and bottom cards
            // only their presence
            if (e.from.pos == asn::Position::Top ||
                e.from.pos == asn::Position::Bottom) {
                const auto &cards = zone(e.from.zone)->cards();
                for (int i = 0; i < cards.size(); ++i) {
                    targets.push_back(&cards[i]);
                }
            } else {
                targets = getTargets(thisCard, e.target, e.from.zone);
            }
            if (targets.empty())
                return false;

            if (e.target.type == asn::TargetType::SpecificCards) {
                const auto &spec = *e.target.targetSpecification;
                if (spec.number.value > static_cast<int>(targets.size()))
                    return false;
            }
        } else if (item.type == asn::EffectType::RemoveMarker) {
            const auto &e = std::get<asn::RemoveMarker>(item.effect);
            assert(e.targetMarker.type == asn::TargetType::SpecificCards);
            assert(e.markerBearer.type == asn::TargetType::ThisCard);
            const auto &spec = *e.targetMarker.targetSpecification;
            auto &markers = thisCard.markers();
            if (markers.size() < spec.number.value)
                return false;

            return true;
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
    if (card.level() > mLevel)
        return false;
    if (card.type() == CardType::Char) {
        if (mCannotPlayBackups)
            return false;
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
    } else if (card.type() == CardType::Event) {
        if (mCannotPlayEvents)
            return false;
        if (card.cost() > zone("stock")->model().count())
            return false;
        return true;
    }

    return false;
}

void Player::interactWithDeck(bool isOpponent) {
    if (!mAbilityList->count())
        return;

    auto &effect = mAbilityList->ability(mAbilityList->activeId()).effect;
    CardZone *pzone;
    if (isOpponent)
        pzone = getOpponent()->zone("deck");
    else
        pzone = zone("deck");
    pzone->visualItem()->setProperty("mGlow", false);
    if (std::holds_alternative<asn::Look>(effect) ||
        std::holds_alternative<asn::RevealCard>(effect)) {
        // deactivate buttons until a new card is revealed
        mAbilityList->activateCancel(mAbilityList->activeId(), false);
        mAbilityList->activatePlay(mAbilityList->activeId(), false);
        sendGameCommand(CommandNextTopDeck());
    } else if (std::holds_alternative<asn::DrawCard>(effect)) {
        mAbilityList->activateCancel(mAbilityList->activeId(), false);
        sendGameCommand(CommandPlayEffect());
        mGame->hideText();
    }
}

void Player::cardInserted(QString startZone, QString targetZone) {
    if (mOpponent) {
        getOpponent()->cardInserted(startZone, targetZone, mOpponent);
        return;
    }
    cardInserted(startZone, targetZone, mOpponent);
}

void Player::cardInserted(QString startZone, QString targetZone, bool isOpponentsCard) {
    if (mOpponent)
        return;

    if (!isOpponentsCard && (startZone == "stock" || targetZone == "stock")) {
        // stock changed, update info
        if (!mResolvingAbilities && mGame->phase() == asn::Phase::MainPhase)
            highlightPlayableCards();
    }

    if (targetZone == "view") {
        if (!mAbilityList->count())
            return;

        auto &a = mAbilityList->ability(mAbilityList->activeId());
        if (std::holds_alternative<asn::Look>(a.effect) ||
            std::holds_alternative<asn::RevealCard>(a.effect)) {
            if (std::holds_alternative<asn::ChooseCard>(a.nextEffect)) {
                const auto &chooseEffect = std::get<asn::ChooseCard>(a.nextEffect);
                if (chooseEffect.targets[0].target.type == asn::TargetType::SpecificCards) {
                    const auto &spec = *chooseEffect.targets[0].target.targetSpecification;
                    if (chooseEffect.targets[0].placeType == asn::PlaceType::Selection) {
                        auto owner = getOpponent(isOpponentsCard);
                        auto from = owner->zone("view");
                        int count = highlightEligibleCards(from, spec.cards.cardSpecifiers, spec.mode, a);

                        asn::NumModifier mod = asn::NumModifier::ExactMatch;
                        if (std::holds_alternative<asn::Look>(a.effect)) {
                            auto &look = std::get<asn::Look>(a.effect);
                            mod = look.number.mod;
                        } else if (std::holds_alternative<asn::RevealCard>(a.effect)) {
                            auto &reveal = std::get<asn::RevealCard>(a.effect);
                            mod = reveal.number.mod;
                        }

                        // in case we HAVE to choose a card, but we have a choice of how much to reveal,
                        // we must have a choice to stop looking
                        if (mod == asn::NumModifier::UpTo && !count)
                            mAbilityList->activateCancel(mAbilityList->activeId(), true);
                    }
                }
            }
        }
    }
}

std::vector<const Card*> Player::getTargets(const Card &thisCard, const asn::Target &t,
                                            asn::Zone from_zone) const {
    std::vector<const Card*> targets;
    if (t.type == asn::TargetType::ThisCard) {
        if (!thisCard.cardPresent())
            return targets;
        targets.push_back(&thisCard);
    } else if (t.type == asn::TargetType::SpecificCards) {
        const auto &spec = *t.targetSpecification;
        const auto &cards = zone(from_zone)->cards();
        for (int i = 0; i < cards.size(); ++i) {
            auto &card = cards[i];
            if (!card.cardPresent())
                continue;

            if (spec.mode == asn::TargetMode::AllOther && thisCard.id() == card.id())
                continue;

            if (checkCard(spec.cards.cardSpecifiers, card))
                targets.push_back(&card);
        }
    } else {
        assert(false);
    }
    return targets;
}
