#pragma once

#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"

class MultiplierComponent;

class CardSpecifierComponent : public BaseComponent
{
    Q_OBJECT
private:
    QQuickItem *parent_;
    QQuickItem *qmlSpecifier_;
    asn::CardSpecifier specifier_;
    QString componentId_;
    QQuickItem *multiplier_area_;
    std::unique_ptr<MultiplierComponent> multiplier_;

public:
    CardSpecifierComponent(QQuickItem *parent, QString componentId);
    ~CardSpecifierComponent();

    void setCardSpecifier(const asn::CardSpecifier& cardSpecifier);

signals:
    void componentReady(const asn::CardSpecifier&, QString);

private slots:
    void valueChanged(QString value);
    void numModifierChanged(QString value);
    void editMultiplier();
    void deleteItself();

    void multiplierChanged(asn::Multiplier multiplier, QString);
    void multiplierClosing();
};

Q_DECLARE_METATYPE(asn::CardSpecifier)
