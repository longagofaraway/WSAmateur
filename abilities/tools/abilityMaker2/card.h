#pragma once

#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"
#include "componentManager.h"

struct CardSpecifier {
    QQuickItem* object;
    asn::CardSpecifier specifier;
};

class CardComponent : public BaseComponent
{
    Q_OBJECT
protected:
    ComponentManager componentManager_;

    QMap<QString, CardSpecifier> cardSpecifiers_;

public:
    CardComponent(QString moduleName, QQuickItem *parent, QString id, QString displayName);

    virtual void createCardSpecifierAdditionalActions() {}
    void notifyOfChanges() override;

    void setCard(const asn::Card&);
    void refitComponents();
    void createCardSpecifier(const asn::CardSpecifier& specifier);

public slots:
    void onCreateCardSpecifier(QString cardSpecifierType, QString value);
    void deleteCardSpecifier(QString componentId);
    void cardSpecifierChanged(const asn::CardSpecifier&, QString);

private:
    void addComponent(QQuickItem* object, QString componentId, const asn::CardSpecifier& specifier);
};

Q_DECLARE_METATYPE(asn::Card)
