#include "player.h"

#include "abilityCommands.pb.h"

#include "abilities.h"
#include "abilityUtils.h"
#include "cardDatabase.h"
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

int Player::highlightCardsForChoice(const asn::Target &target, const asn::Place &place) {
    const auto &specs = target.targetSpecification->cards.cardSpecifiers;
    int eligibleCount = 0;
    if (place.owner == asn::Player::Player || place.owner == asn::Player::Both) {
        auto from = zone(asnZoneToString(place.zone));
        eligibleCount += highlightEligibleCards(from, specs, target.targetSpecification->mode, mAbilityList->ability(mAbilityList->activeId()));
    }
    if (place.owner == asn::Player::Opponent || place.owner == asn::Player::Both) {
        auto from = getOpponent()->zone(asnZoneToString(place.zone));
        eligibleCount += highlightEligibleCards(from, specs, target.targetSpecification->mode, mAbilityList->ability(mAbilityList->activeId()));
    }
    return eligibleCount;
}

void Player::processChooseCardInternal(int eligibleCount, OptionalPlace place, bool mandatory, asn::Player executor) {
    if (eligibleCount) {
        if (place && place->get().zone == asn::Zone::WaitingRoom &&
            place->get().owner == asn::Player::Player)
            QMetaObject::invokeMethod(zone("wr")->visualItem(), "openView", Q_ARG(QVariant, true));
        if (!mandatory)
            mAbilityList->activateCancel(mAbilityList->activeId(), true);
    } else {
        mGame->pause(500);
        if (executor == asn::Player::Player)
            sendGameCommand(CommandCancelEffect());
        else
            getOpponent()->sendGameCommand(CommandCancelEffect());
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
                cmd.add_positions(from->model().count() - 1 - i);
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
        auto from = getOpponent()->zone(asnZoneToString(place->get().zone));
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
    CardZone *pzone;
    if (!opponent)
        pzone = zone(qzone.toStdString());
    else
        pzone = mGame->opponent()->zone(qzone.toStdString());

    // someday we'll need to count selected and highlighted in all needed zones
    int selected = pzone->numOfSelectedCards();
    int highlighted = pzone->numOfHighlightedCards();

    ActivatedAbility *a;
    if (hasActivatedAbilities())
        a = &activeAbility();
    else
        a = &getOpponent()->activeAbility();
    auto &effect = a->effect;
    asn::Number number;
    if (std::holds_alternative<asn::ChooseCard>(effect) || std::holds_alternative<asn::Look>(effect)) {
        const auto &ef = std::holds_alternative<asn::ChooseCard>(effect) ?
                    std::get<asn::ChooseCard>(effect) :
                    std::get<asn::ChooseCard>(a->nextEffect);
        assert(ef.targets.size() == 1);
        if (ef.targets[0].type != asn::TargetType::SpecificCards)
            return;

        number = ef.targets[0].targetSpecification->number;
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
    }

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
                cmd.add_positions(zone("deck")->model().count() - 1 - i);
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
            if (e.from.zone == asn::Zone::Hand && e.target.type == asn::TargetType::SpecificCards &&
                e.target.targetSpecification->number.value > zone("hand")->model().count())
                return false;
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
    if (card.type() == CardType::Event)
        return !mCannotPlayEvents;
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

std::vector<const Card*> Player::getTargets(const Card &thisCard, const asn::Target &t) const {
    std::vector<const Card*> targets;
    if (t.type == asn::TargetType::ThisCard) {
        if (!thisCard.cardPresent())
            return targets;
        targets.push_back(&thisCard);
    } else if (t.type == asn::TargetType::SpecificCards) {
        const auto &spec = *t.targetSpecification;
        const auto &cards = mStage->cards();
        for (int i = 0; i < cards.size(); ++i) {
            auto &card = cards[i];
            if (!card.cardPresent())
                continue;

            if (spec.mode == asn::TargetMode::AllOther && thisCard.id() != card.id())
                continue;

            if (checkCard(spec.cards.cardSpecifiers, card))
                targets.push_back(&card);
        }
    } else {
        assert(false);
    }
    return targets;
}
