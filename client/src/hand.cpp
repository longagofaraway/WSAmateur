#include "hand.h"

#include <QQmlContext>

#include "card.h"
#include "player.h"
#include "game.h"

Hand::Hand(Player *player, Game *game)
    : mPlayer(player), mGame(game) {
    QQmlComponent component(mGame->engine(), "qrc:/qml/Hand.qml");
    QQmlContext *context = new QQmlContext(mGame->context(), mGame);
    context->setContextProperty("innerModel", QVariant::fromValue(&mCardsModel));
    QObject *obj = component.create(context);
    mQmlHand = qobject_cast<QQuickItem*>(obj);
    mQmlHand->setParentItem(mGame);
    mQmlHand->setParent(mGame);
    mQmlHand->setProperty("opponent", player->isOpponent());
}

void Hand::addCard() {
    mCardsModel.addCard();
}

void Hand::addCard(const std::string &code) {
    mCardsModel.addCard(code);
}

void Hand::startMulligan() {
    QMetaObject::invokeMethod(mQmlHand, "mulligan");
}

void Hand::endMulligan() {
    QMetaObject::invokeMethod(mQmlHand, "endMulligan");
}

void Hand::clockPhase() {
    QMetaObject::invokeMethod(mQmlHand, "clockPhase");
}

void Hand::endClockPhase() {
    QMetaObject::invokeMethod(mQmlHand, "endClockPhase");
}

void Hand::mainPhase() {
    mQmlHand->setProperty("state", "main");
}

void Hand::endMainPhase() {
    mQmlHand->setProperty("state", "");
    QMetaObject::invokeMethod(mQmlHand, "glowAllCards", Q_ARG(QVariant, false));
}

void Hand::discardCard() {
    QMetaObject::invokeMethod(mQmlHand, "discardCard");
}

void Hand::deactivateDiscarding() {
}

