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
    QQuickItem *abilityText;

    std::shared_ptr<BaseComponent> currentComponent;

public:
    Q_INVOKABLE QString translate(const asn::Ability &ability);

protected:
    void componentComplete() override;
};
