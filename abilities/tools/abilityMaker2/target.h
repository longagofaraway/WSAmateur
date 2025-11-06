#pragma once

#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"
#include "componentManager.h"

struct CardSpecifier {
    QQuickItem* object;
    asn::CardSpecifier specifier;
};

class TargetComponent : public BaseComponent
{
    Q_OBJECT
private:
    ComponentManager componentManager_;

    asn::TargetType type_;
    asn::TargetMode mode_{asn::TargetMode::Any};
    asn::Number number_;
    QMap<QString, CardSpecifier> cardSpecifiers_;

public:
    TargetComponent(QQuickItem *parent, QString id, QString displayName);

    void setTarget(const asn::Target&);

signals:
    void targetReady(const asn::Target&);

private slots:
    void onCreateCardSpecifier(QString cardSpecifierType, QString value);
    void deleteCardSpecifier(QString componentId);
    void onNumModifierChanged(QString);
    void onNumValueChanged(QString);
    void setTargetMode(QString);
    void setTargetType(QString);
    void cardSpecifierChanged(const asn::CardSpecifier&, QString);

private:
    void createCardSpecifier(const asn::CardSpecifier& specifier);
    void addComponent(QQuickItem* object, QString componentId, const asn::CardSpecifier& specifier);
    void refitComponents();
    void buildTarget();

    void setSecondLine();
};

Q_DECLARE_METATYPE(asn::Target)
