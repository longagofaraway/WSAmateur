#include "stage.h"

#include <QQmlContext>

#include "player.h"
#include "game.h"

#include <QDebug>

Stage::Stage(Player *player, Game *game)
    : CardZone(player), mGame(game) {
    mName = "stage";
    mCardsModel.addCards(5, this);

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

void Stage::setAttr(int row, ProtoCardAttribute attr, int value) {
    mCardsModel.setAttr(row, attr, value);
    switch (attr) {
    case ProtoAttrPower:
        QMetaObject::invokeMethod(mQmlObject, "powerChangeAnim", Q_ARG(QVariant, row));
        break;
    case ProtoAttrSoul:
        QMetaObject::invokeMethod(mQmlObject, "soulChangeAnim", Q_ARG(QVariant, row));
        break;
    case ProtoAttrLevel:
        QMetaObject::invokeMethod(mQmlObject, "levelChangeAnim", Q_ARG(QVariant, row));
        break;
    default:
        assert(false);
        break;
    }
}

void Stage::mainPhase() {
    mQmlObject->connect(mQmlObject, SIGNAL(switchPositions(int,int)), mPlayer, SLOT(sendSwitchPositions(int,int)));
    mQmlObject->setProperty("mDragEnabled", true);
}

void Stage::endMainPhase() {
    mQmlObject->disconnect(mQmlObject, SIGNAL(switchPositions(int, int)), mPlayer, SLOT(sendSwitchPositions(int, int)));
    mQmlObject->setProperty("mDragEnabled", false);
}

void Stage::attackDeclarationStep() {
    mQmlObject->setProperty("state", "attack");
    mQmlObject->connect(mQmlObject, SIGNAL(declareAttack(int,bool)), mPlayer, SLOT(sendAttackDeclaration(int,bool)));
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
        if (cards[i].cardPresent() && cards[i].state() == asn::State::Standing)
            mCardsModel.setGlow(i, highlight);
    }
}

void Stage::attackDeclared(int pos) {
    mGame->pause(500);

    mCardsModel.setSelected(pos, true);
    mCardsModel.setState(pos, asn::State::Rested);

    if (mPlayer->isOpponent())
        return;

    mQmlObject->disconnect(mQmlObject, SIGNAL(declareAttack(int, bool)), mPlayer, SLOT(sendAttackDeclaration(int, bool)));
    mQmlObject->setProperty("state", "");
    auto &cards = mCardsModel.cards();
    for (int i = 0; i < 3; ++i) {
        if (cards[i].cardPresent() && i != pos)
            mCardsModel.setGlow(i, false);
    }
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
        if (cards[i].cardPresent() && cards[i].state() == asn::State::Reversed) {
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

void Stage::swapCards(int from, int to) {
    QMetaObject::invokeMethod(mQmlObject, "swapCards", Q_ARG(QVariant, from), Q_ARG(QVariant, to));
}

std::tuple<qreal, qreal> Stage::coords(int pos) {
    QVariant qx, qy;
    QMetaObject::invokeMethod(mQmlObject, "getXForNewCard", Q_RETURN_ARG(QVariant, qx), Q_ARG(QVariant, pos));
    QMetaObject::invokeMethod(mQmlObject, "getYForNewCard", Q_RETURN_ARG(QVariant, qy), Q_ARG(QVariant, pos));
    return {qx.toReal(), qy.toReal()};
}
