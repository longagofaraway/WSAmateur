#pragma once

#include <vector>

#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"

class AbilityComponent;

class ArrayOfAbilitiesComponent : public BaseComponent
{
    Q_OBJECT
private:
    std::vector<QQuickItem*> qmlAbilities;
    std::vector<asn::Ability> abilities;
    std::vector<bool> abilitiesSet;

    std::shared_ptr<AbilityComponent> qmlAbility;

    int currentPos = 0;

    bool onlyEventAbilities = false;
    bool onlyAutoAbilities = false;

public:
    ArrayOfAbilitiesComponent(QQuickItem *parent);
    ArrayOfAbilitiesComponent(const std::vector<asn::Ability> &ab, QQuickItem *parent);

    void fixEventAbility() { onlyEventAbilities = true; }
    void fixAutoAbility() { onlyAutoAbilities = true; }

signals:
    void componentChanged(const std::vector<asn::Ability> &a);

private slots:
    void addAbility();
    void editAbility(int pos);

    void destroyAbility();
    void abilityReady(const asn::Ability &ability);

private:
    void componentReady() override;
    void init();
    void createAbility();
};

