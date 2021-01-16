#pragma once

#include <QObject>
#include <QQuickItem>

#include <QDebug>

class AbilityMaker : public QQuickItem {
    Q_OBJECT
public:
    Q_INVOKABLE void createAbility();
    Q_INVOKABLE QString createComponent(QString componentName);
};
