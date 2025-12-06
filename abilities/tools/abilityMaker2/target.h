#pragma once

#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"
#include "card.h"
#include "componentManager.h"


class TargetComponent : public CardComponent
{
    Q_OBJECT
private:
    asn::TargetType type_;
    asn::TargetMode mode_{asn::TargetMode::Any};
    asn::Number number_;

public:
    TargetComponent(QQuickItem *parent, QString id, QString displayName);

    void createCardSpecifierAdditionalActions() override;

    void setTarget(const asn::Target&);
    void notifyOfChanges() override;

private slots:
    void onNumModifierChanged(QString);
    void onNumValueChanged(QString);
    void setTargetMode(QString);
    void setTargetType(QString);

private:
    void addComponent(QQuickItem* object, QString componentId, const asn::CardSpecifier& specifier);

    void setSecondLine();
};

Q_DECLARE_METATYPE(asn::Target)
