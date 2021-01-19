#include "stage.h"

#include <QQmlContext>

#include "player.h"
#include "game.h"

#include <QDebug>

Stage::Stage(Player *player, Game *game)
    : mPlayer(player), mGame(game) {
    mCardsModel.addCards(5);

    QQmlComponent component(mGame->engine(), "qrc:/qml/Stage.qml");
    QQmlContext *context = new QQmlContext(mGame->context(), mGame);
    context->setContextProperty("innerModel", QVariant::fromValue(&mCardsModel));
    QObject *obj = component.create(context);
    mQmlObject = qobject_cast<QQuickItem*>(obj);
    mQmlObject->setParentItem(mGame);
    mQmlObject->setParent(mGame);
    mQmlObject->setProperty("opponent", player->isOpponent());
    mQmlObject->connect(mQmlObject, SIGNAL(sendToWr(int)), mPlayer, SLOT(sendFromStageToWr(int)));
}

void Stage::mainPhase() {
    mQmlObject->connect(mQmlObject, SIGNAL(switchPositions(int, int)), mPlayer, SLOT(switchPositions(int, int)));
    mQmlObject->setProperty("mDragEnabled", true);
}

void Stage::endMainPhase() {
    mQmlObject->disconnect(mQmlObject, SIGNAL(switchPositions(int, int)), mPlayer, SLOT(switchPositions(int, int)));
    mQmlObject->setProperty("mDragEnabled", false);
}

void Stage::attackDeclarationStep() {
    mQmlObject->setProperty("state", "attack");
    mQmlObject->connect(mQmlObject, SIGNAL(declareAttack(int, bool)), mPlayer, SLOT(sendAttackDeclaration(int, bool)));
    unhighlightAttacker();
    highlightAttackers(true);
}

void Stage::unhighlightAttacker() {
    auto &cards = mCardsModel.cards();
    for (int i = 0; i < 3; ++i) {
        if (cards[i].cardPresent() && cards[i].selected()) {
            mCardsModel.setSelected(i, false);
            mCardsModel.setGlow(i, false);
        }
    }
}

void Stage::highlightAttackers(bool highlight) {
    auto &cards = mCardsModel.cards();
    for (int i = 0; i < 3; ++i) {
        if (cards[i].cardPresent() && cards[i].state() == StateStanding)
            mCardsModel.setGlow(i, highlight);
    }
}

void Stage::attackDeclared(int pos) {
    QMetaObject::invokeMethod(mQmlObject, "attackDeclared", Q_ARG(QVariant, pos));

    mCardsModel.setSelected(pos, true);

    if (mPlayer->isOpponent())
        return;

    mCardsModel.setState(pos, StateRested);
    mQmlObject->setProperty("state", "");
    mQmlObject->disconnect(mQmlObject, SIGNAL(declareAttack(int, bool)), mPlayer, SLOT(sendAttackDeclaration(int, bool)));
    auto &cards = mCardsModel.cards();
    for (int i = 0; i < 3; ++i) {
        if (cards[i].cardPresent() && i != pos)
            mCardsModel.setGlow(i, false);
    }
}

namespace {
int qmlCardState(CardState state) {
    switch (state) {
    case StateStanding:
        return 0;
    case StateRested:
        return 1;
    case StateReversed:
        return 2;
    }
    assert(false);
    return 0;
}
}

void Stage::setCardState(int pos, CardState state) {
    mCardsModel.setState(pos, StateRested);
    QMetaObject::invokeMethod(mQmlObject, "setCardState", Q_ARG(QVariant, pos), Q_ARG(QVariant, qmlCardState(state)));
}

void Stage::endAttackPhase() {
    mQmlObject->disconnect(mQmlObject, SIGNAL(declareAttack(int, bool)), mPlayer, SLOT(sendAttackDeclaration(int, bool)));
    mQmlObject->setProperty("state", "");
    unhighlightAttacker();
    highlightAttackers(false);
}

void Stage::encoreStep() {
    unhighlightAttacker();
    auto &cards = mCardsModel.cards();
    for (int i = 0; i < 5; ++i) {
        if (cards[i].cardPresent() && cards[i].state() == StateReversed) {
            mCardsModel.setGlow(i, true);
        }
    }
    mQmlObject->setProperty("state", "encore");
    mQmlObject->connect(mQmlObject, SIGNAL(encoreCharacter(int)), mPlayer, SLOT(sendEncore(int)));
}

void Stage::deactivateEncoreStep() {
    auto &cards = mCardsModel.cards();
    for (int i = 0; i < 5; ++i) {
        if (cards[i].cardPresent()) {
            mCardsModel.setGlow(i, false);
        }
    }
    mQmlObject->setProperty("state", "");
    mQmlObject->disconnect(mQmlObject, SIGNAL(encoreCharacter(int)), mPlayer, SLOT(sendEncore(int)));
}
