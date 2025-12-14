#pragma once

#include <QQuickItem>

#include "abilities.h"
#include "language_parser.h"

using VarTrigger = decltype(asn::Trigger::trigger);
using VarEffect = decltype(asn::Effect::effect);

class BaseComponent : public QObject
{
    Q_OBJECT
protected:
    QQuickItem *qmlObject_;
    QString componentId_;

public:
    explicit BaseComponent(const QString &moduleName, QQuickItem *parent);
    explicit BaseComponent(const QString &moduleName, QQuickItem *parent, QString componentId);
    virtual ~BaseComponent();

    void init(const QString &moduleName, QQuickItem *parent, QString compnentId);

    QQuickItem* getQmlObject() { return qmlObject_; }

    virtual void notifyOfChanges() { throw std::runtime_error("not implemented"); }
    virtual asn::TriggerType getLanguageComponentType(formats::To<asn::TriggerType>) { throw std::runtime_error("not implemented"); }
    virtual VarTrigger& getLanguageComponent(formats::To<VarTrigger>) { throw std::runtime_error("not implemented"); }
    virtual asn::EffectType getLanguageComponentType(formats::To<asn::EffectType>) { throw std::runtime_error("not implemented"); }
    virtual VarEffect& getLanguageComponent(formats::To<VarEffect>) { throw std::runtime_error("not implemented"); }

signals:
    void close();

    void deleteComponent(QString componentId);
    void setCard(const asn::Card&, QString);
    void setCardSpecifier(const asn::CardSpecifier&, QString);
    void setTarget(const asn::Target&, QString);
    void setTargetAndPlace(const asn::TargetAndPlace&, QString);
    void setZone(QString, QString);
    void setPhase(QString, QString);
    void setPhaseState(QString, QString);
    void setPlayer(QString, QString);
    void setState(QString, QString);
    void setAttackType(QString, QString);
    void setAbilityType(QString, QString);
    void setAttributeType(QString, QString);
    void setBool(bool, QString);
    void setString(QString, QString);
    void setValueType(QString, QString);
    void setInt32(QString, QString);
    void setInt8(QString, QString);
    void setUInt8(QString, QString);
    void setMultiplier(const std::optional<asn::Multiplier>&, QString);
    void setDuration(int, QString);
    void setNumber(const asn::Number&, QString);
    void setPlace(const asn::Place&, QString);
    void setRevealType(QString, QString);
    void setOrder(QString, QString);
    void setEffect(const asn::Effect&, QString);
    void setAbility(const asn::Ability&, QString);
    void setEventAbility(const asn::EventAbility&, QString);
    void setAutoAbility(const asn::AutoAbility&, QString);
    void setSearchTarget(const asn::SearchTarget&, QString);
    void setBackupOrEvent(QString, QString);
    void setChooseCard(const asn::ChooseCard&, QString);
    void setFaceOrientation(QString, QString);
    void setPosition(QString, QString);
    void setTriggerIcon(QString, QString);
    void setTraitModificationType(QString, QString);
    void setTraitType(QString, QString);

    void targetReady(const asn::Target&, QString);
    void cardReady(const asn::Card&, QString);
};

