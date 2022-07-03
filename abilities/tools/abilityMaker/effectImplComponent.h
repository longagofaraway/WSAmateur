#pragma once

#include <QQuickItem>

#include "abilities.h"
#include "cardComponent.h"
#include "chooseCardComponent.h"
#include "multiplierComponent.h"
#include "placeComponent.h"
#include "targetComponent.h"

class ArrayOfEffectsComponent;
class ArrayOfAbilitiesComponent;

class EffectImplComponent : public QObject
{
    Q_OBJECT
public:
    using VarEffect = decltype(asn::Effect::effect);

private:
    QQuickItem *qmlObject = nullptr;

    asn::EffectType type;
    VarEffect effect;

    std::unique_ptr<TargetComponent> qmlTarget;

    std::unique_ptr<PlaceComponent> qmlPlace;
    std::unique_ptr<ChooseCardComponent> qmlChooseCard;
    std::unique_ptr<CardComponent> qmlCard;
    std::unique_ptr<MultiplierComponent> qmlMultiplier;
    std::unique_ptr<ArrayOfEffectsComponent> qmlEffects;
    std::unique_ptr<ArrayOfAbilitiesComponent> qmlAbilities;

    int currentCardIndex = 0;

public:
    EffectImplComponent(asn::EffectType type, QQuickItem *parent);
    EffectImplComponent(asn::EffectType type, const VarEffect &e, QQuickItem *parent);
    ~EffectImplComponent();

signals:
    void componentChanged(const VarEffect &e);

private slots:
    void editTarget(std::optional<asn::Target> target_ = {});
    void destroyTarget();
    void targetReady(const asn::Target &t);
    void secondTargetReady(const asn::Target &t);
    void editMarkerBearer();
    void editDestination();

    void onAttrTypeChanged(int value);
    void onAttrChanged(QString value);
    void onDurationChanged(int value);

    void editPlace(std::optional<asn::Place> place = {});
    void editTo();
    void editTo2();
    void addDestination();
    void destroyPlace();
    void placeReady(const asn::Place &p);
    void placeToReady(const asn::Place &p);

    void onPlaceTypeChanged(int value);
    void onPlayerChanged(int value);
    void onZoneChanged(int value);
    void onPositionChanged(int value);
    void onPhaseChanged(int value);

    void editCard();
    void cardReady(const asn::Card &card_);
    void destroyCard();

    void onRevealTypeChanged(int value);

    void onNumModifierChanged(int value);
    void onNumValueChanged(QString value);

    void onValueTypeChanged(int value);
    void editMultiplier();
    void destroyMultiplier();
    void multiplierReady(const asn::Multiplier &m);

    void onOrderChanged(int value);

    void editEffectsField();
    void editIfYouDo();
    void editIfYouDont();
    void editEffects(const std::vector<asn::Effect> &effects);
    void effectsReady(const std::vector<asn::Effect> &effects);
    void ifYouDoReady(const std::vector<asn::Effect> &effects);
    void ifYouDontReady(const std::vector<asn::Effect> &effects);
    void destroyEffects();

    void editAbilities();
    void destroyAbilities();
    void abilitiesReady(const std::vector<asn::Ability> &a);

    void onNumOfTimesChanged(QString value);

    void onCardStateChanged(int value);

    void onBackupLevelChanged(QString value);
    void onBackupOrEventChanged(int value);

    void onAttackTypeChanged(int value);
    void onFaceOrientationChanged(int value);

    void editSwapCards(int n);
    void editChooseOne();
    void editChooseTwo();
    void destroyChooseCard();
    void chooseOneReady(const asn::ChooseCard &e);
    void chooseTwoReady(const asn::ChooseCard &e);

    void onTriggerIconChanged(int value);

    void cardCodeChanged(QString code);

private:
    void init(QQuickItem *parent);
};

void initEffectByType(EffectImplComponent::VarEffect &effect, asn::EffectType type);

