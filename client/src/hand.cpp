#include "hand.h"

#include <QQmlContext>

#include "card.h"
#include "player.h"
#include "game.h"

Hand::Hand(Player *player, Game *game)
    : CardZone(player), mGame(game) {
    mName = "hand";
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
    mCardsModel.addCard(this);
}

void Hand::addCard(const std::string &code) {
    mCardsModel.addCard(code, this);
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

void Hand::playTiming() {
    mQmlHand->setProperty("state", "playTiming");
}

void Hand::endPlayTiming() {
    mQmlHand->setProperty("state", "");
    QMetaObject::invokeMethod(mQmlHand, "glowAllCards", Q_ARG(QVariant, false));
}

void Hand::discardCard() {
    QMetaObject::invokeMethod(mQmlHand, "discardCard");
}

void Hand::deactivateDiscarding() {
}

bool Hand::isPlayTiming() {
    return mQmlHand->property("state").toString() == "playTiming" ? true : false;
}

