#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "activatedAbilityModel.h"
#include "cardModel.h"
#include "choiceDialog.h"
#include "game.h"
#include "player.h"
#include "imageProvider.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES, true);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    qmlRegisterType<Game>("wsamateur", 1, 0, "Game");
    qmlRegisterUncreatableType<Player>("wsamateur", 1, 0, "Player", "Player cannot be created in QML");
    qmlRegisterType<CardModel>("wsamateur", 1, 0, "CardModel");
    qmlRegisterType<ActivatedAbilityModel>("wsamateur", 1, 0, "ActivatedAbilityModel");
    qmlRegisterType<ChoiceDialogModel>("wsamateur", 1, 0, "ChoiceDialogModel");
    qmlRegisterType<AbilityModel>("wsamateur", 1, 0, "AbilityModel");

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
