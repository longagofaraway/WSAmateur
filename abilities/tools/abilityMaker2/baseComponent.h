#pragma once

#include <QQuickItem>

#include "abilities.h"

class BaseComponent : public QObject
{
    Q_OBJECT
protected:
    QQuickItem *qmlObject;

public:
    explicit BaseComponent(const QString &moduleName, QQuickItem *parent);
    explicit BaseComponent(const QString &moduleName, QQuickItem *parent, QString componentId);
    virtual ~BaseComponent();

    void init(const QString &moduleName, QQuickItem *parent, QString compnentId);

    QQuickItem* getQmlObject() { return qmlObject; }

signals:
    void close();

    void deleteComponent(QString componentId);
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

/*public slots:
    virtual void zoneChanged(QString) = 0;
    virtual void zone2Changed(QString) = 0;*/
};

