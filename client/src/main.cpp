#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QFontDatabase>
#include <QIcon>

#include "activatedAbilityModel.h"
#include "application.h"
#include "cardModel.h"
#include "choiceDialog.h"
#include "deckMenu.h"
#include "deckMenuModel.h"
#include "game.h"
#include "userListModel.h"
#include "lobby.h"
#include "player.h"
#include "imageProvider.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES, true);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/resources/icon.ico"));

    QFontDatabase::addApplicationFont(":/resources/fonts/Futura BK BT.ttf");
    QFontDatabase::addApplicationFont(":/resources/fonts/Aprikas_black_Demo.otf");

    qmlRegisterType<WSApplication>("wsamateur", 1, 0, "WSApplication");
    qmlRegisterType<Game>("wsamateur", 1, 0, "Game");
    qmlRegisterUncreatableType<Player>("wsamateur", 1, 0, "Player", "Player cannot be created in QML");
    qmlRegisterType<CardModel>("wsamateur", 1, 0, "CardModel");
    qmlRegisterType<ActivatedAbilityModel>("wsamateur", 1, 0, "ActivatedAbilityModel");
    qmlRegisterType<ChoiceDialogModel>("wsamateur", 1, 0, "ChoiceDialogModel");
    qmlRegisterType<AbilityModel>("wsamateur", 1, 0, "AbilityModel");
    qmlRegisterType<Lobby>("wsamateur", 1, 0, "Lobby");
    qmlRegisterType<DeckMenu>("wsamateur", 1, 0, "DeckMenu");
    qmlRegisterType<UserListModel>("wsamateur", 1, 0, "UserListModel");
    qmlRegisterType<DeckMenuModel>("wsamateur", 1, 0, "DeckMenuModel");

    qRegisterMetaType<uint16_t>("uint16_t");

    QQmlApplicationEngine engine;

    engine.addImageProvider("imgprov", new AsyncImageProvider);

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
