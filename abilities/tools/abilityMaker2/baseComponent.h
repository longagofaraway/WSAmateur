#pragma once

#include <QQuickItem>

#include "abilities.h"
#include "language_parser.h"

using VarTrigger = decltype(asn::Trigger::trigger);

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

signals:
    void close();

    void deleteComponent(QString componentId);
    void setCard(const asn::Card&, QString);
    void setCardSpecifier(const asn::CardSpecifier&, QString);
    void setTarget(const asn::Target&, QString);
    void setZone(QString, QString);
    void setPhase(QString, QString);
    void setPhaseState(QString, QString);
    void setPlayer(QString, QString);
    void setState(QString, QString);
    void setAttackType(QString, QString);
    void setAbilityType(QString, QString);
    void setBool(bool, QString);
    void setString(QString, QString);

    void targetReady(const asn::Target&, QString);
};

