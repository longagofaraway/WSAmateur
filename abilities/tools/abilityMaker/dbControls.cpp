#include "dbControls.h"

#include <QDir>
#include <QQmlContext>
#include <QStandardPaths>

#include "abilityMaker.h"

DbControls::DbControls(AbilityMaker *maker_, QQuickItem *parent)
    : maker(maker_){
    QQmlComponent component(qmlEngine(parent), "qrc:/qml/DbControls.qml");
    QQmlContext *context = new QQmlContext(qmlContext(parent), parent);
    QObject *obj = component.create(context);
    QQuickItem *qmlObject = qobject_cast<QQuickItem*>(obj);
    qmlObject->setParentItem(parent);
    qmlObject->setParent(parent);
    // parent's height is negative wtf, so set the constant
    qmlObject->setProperty("y", 300);

    connect(qmlObject, SIGNAL(addAbility(QString)), this, SLOT(addAbility(QString)));
    connect(qmlObject, SIGNAL(popAbility(QString)), this, SLOT(popAbility(QString)));
    connect(this, SIGNAL(sendMessage(QString)), qmlObject, SIGNAL(setStatus(QString)));

    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir path(appData);
    path.cdUp();
    path.cd("WSAmateur");

    try {
        dbManager = std::make_unique<DbManager>(path.filePath("cards.db"));
    } catch (const std::exception &e) {
        emit sendMessage(e.what());
    }
}

DbControls::~DbControls() {
    //qmlObject->deleteLater();
}

void DbControls::addAbility(QString code) {
    try {
        dbManager->addAbility(code, maker->getAbility());
        emit sendMessage("Ability added");
    } catch (const std::exception &e) {
        emit sendMessage(e.what());
    }
}

void DbControls::popAbility(QString code) {
    try {
        dbManager->popAbility(code);
        emit sendMessage("Ability removed");
    } catch (const std::exception &e) {
        emit sendMessage(e.what());
    }
}
