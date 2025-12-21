#pragma once

#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"

class NumberComponent : public BaseComponent
{
    Q_OBJECT
private:
    asn::Number number_;

public:
    NumberComponent(QQuickItem *parent, QString id, QString displayName);

    void setNumber(asn::Number);
    void notifyOfChanges() override;

public slots:
    void numModifierChanged(QString numModifier);
    void valueChanged(QString value);
};
