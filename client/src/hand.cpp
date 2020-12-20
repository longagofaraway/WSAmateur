#include "hand.h"

#include <QQmlContext>

#include "card.h"
#include "player.h"
#include "game.h"

Hand::Hand(Player *player, Game *game)
    : mPlayer(player), mGame(game) {
    /*mCardsModel = {
        new HandCard("IMC/W43-111", false),
        new HandCard("IMC/W43-009", false),
        new HandCard("IMC/W43-046", true),
        new HandCard("IMC/W43-127", false),
        new HandCard("IMC/W43-091", false),
        new HandCard("IMC/W43-046", false),
        new HandCard("IMC/W43-009", false)
    };*/

    QQmlComponent component(mGame->engine(), "qrc:/qml/Hand.qml");
    QQmlContext *context = new QQmlContext(mGame->context(), mGame);
    context->setContextProperty("innerModel", QVariant::fromValue(&mCardsModel));
    QObject *obj = component.create(context);
    mQmlHand = qobject_cast<QQuickItem*>(obj);
    mQmlHand->setParentItem(mGame);
    mQmlHand->setParent(mGame);
    mQmlHand->setProperty("opponent", player->isOpponent());
}

std::vector<Card> &Hand::cards() {
    return mCardsModel.cards();
}

void Hand::addCard() {
    mCardsModel.addCard();
}

void Hand::addCard(const std::string &code) {
    mCardsModel.addCard(code);
}

void Hand::startMulligan() {
    mQmlHand->setState("mulligan");
    mQmlHand->connect(mQmlHand, SIGNAL(cardSelected(bool)), mGame, SLOT(cardSelectedForMulligan(bool)));
}

void Hand::endMulligan() {
    mQmlHand->setState("");
    mQmlHand->disconnect(mQmlHand, SIGNAL(cardSelected(bool)), mGame, SLOT(cardSelectedForMulligan(bool)));
}

void Hand::clockPhase() {
    QMetaObject::invokeMethod(mQmlHand, "clockPhase");
    mQmlHand->connect(mQmlHand, SIGNAL(cardSelected(bool)), mGame, SLOT(cardSelectedForClock(bool)));
}

void Hand::endClockPhase() {
    QMetaObject::invokeMethod(mQmlHand, "endClockPhase");
    mQmlHand->disconnect(mQmlHand, SIGNAL(cardSelected(bool)), mGame, SLOT(cardSelectedForClock(bool)));
}

void Hand::mainPhase() {
    //QMetaObject::invokeMethod(mQmlHand, "clockPhase");
    mQmlHand->setProperty("state", "main");
    mQmlHand->connect(mQmlHand, SIGNAL(cardPlayed(int, int)), mPlayer, SLOT(cardPlayed(int, int)));
}

