#pragma once

#include <QObject>
#include <QQuickItem>
#include <QString>

#include <QDebug>

#include "baseComponent.h"

class AbilityMaker : public QQuickItem {
    Q_OBJECT
private:
    QQuickItem *abilityMenu;
    QQuickItem *workingArea;

    std::shared_ptr<BaseComponent> qmlObject;

private slots:
    void createTrigger(QString triggerId);

protected:
    void componentComplete() override;
};
