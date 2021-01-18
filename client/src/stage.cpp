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
    highlightAttackers();
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

void Stage::highlightAttackers() {
    auto &cards = mCardsModel.cards();
    for (int i = 0; i < 3; ++i) {
        if (cards[i].cardPresent() && cards[i].state() == StateStanding)
            mCardsModel.setGlow(i, true);
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
