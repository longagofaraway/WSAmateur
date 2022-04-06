#pragma once

#include <QObject>
#include <QQuickItem>

#include <QDebug>

#include "abilityComponent.h"

class AbilityMaker : public QQuickItem {
    Q_OBJECT
private:
    std::unique_ptr<AbilityComponent> qmlAbility;
    asn::Ability ability_;
    QString abilityPath;

public:
    ~AbilityMaker();

    asn::Ability getAbility() const { return ability_; }
    void setAbility(const asn::Ability &a);
    void statusLinePush(QString dir);
    void statusLinePop();

signals:
    void updateStatusLine(QString line);

public slots:
    void translate(const asn::Ability &ability);

protected:
    void componentComplete() override;
};
