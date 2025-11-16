#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "ability.h"
#include "abilityMaker.h"
#include "effectsTree.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<AbilityMaker>("abilityMaker", 1, 0, "AbilityMaker");
    qmlRegisterType<EffectsTree>("effectsTree", 1, 0, "EffectsTree");
    qmlRegisterType<AbilityComponent>("abilityComponent", 1, 0, "AbilityComponent");
    qRegisterMetaType<asn::Ability>();

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
