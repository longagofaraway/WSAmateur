#pragma once

#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"

class CardSpecifierComponent : public BaseComponent
{
    Q_OBJECT
private:
    QQuickItem *qmlSpecifier_;
    asn::CardSpecifier specifier_;
    QString componentId_;

public:
    CardSpecifierComponent(QQuickItem *parent, QString componentId);
    ~CardSpecifierComponent();

    void setCardSpecifier(const asn::CardSpecifier& cardSpecifier);

signals:
    void componentReady(const asn::CardSpecifier&, QString);

private slots:
    void valueChanged(QString value);
    void numModifierChanged(QString value);
    void deleteItself();
};

Q_DECLARE_METATYPE(asn::CardSpecifier)
