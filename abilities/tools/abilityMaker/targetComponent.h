#pragma once

#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"
#include "cardComponent.h"

class TargetComponent : public BaseComponent
{
    Q_OBJECT
private:
    asn::TargetType type;
    asn::TargetMode mode{};
    asn::Number number{};
    std::unique_ptr<CardComponent> qmlCard;
    asn::Card card;
    bool cardSet = false;

public:
    explicit TargetComponent(QQuickItem *parent);
    TargetComponent(const asn::Target &t, QQuickItem *parent);

signals:
    void componentChanged(const asn::Target &t);
    void initTargetType(int type);
    void initTargetMode(int type);
    void initNumModifier(int type);
    void initNumValue(QString value);

private slots:
    void updateTarget(int index);
    void typeChanged(int index);
    void targetModeChanged(int index);
    void numModifierChanged(int index);
    void valueChanged(QString value);
    void editCard();
    void clearCard();
    void cardReady(const asn::Card &card_);
    void destroyCard();

private:
    void init();
    void initTarget(const asn::Target &t);
    void componentReady() override;
    asn::Target constructTarget();
};

