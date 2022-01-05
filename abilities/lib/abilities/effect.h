#pragma once

#include <variant>
#include <optional>

#include "condition.h"
#include "multiplier.h"
#include "number.h"
#include "target.h"

namespace asn {

struct Ability;
struct EventAbility;
struct Effect;

struct TargetAndPlace {
    Target target;
    PlaceType placeType;
    std::optional<Place> place;
};

struct ChooseCard {
    Player executor;
    std::vector<TargetAndPlace> targets;
    std::vector<Card> excluding;
};

enum class ValueType : uint8_t {
    Raw = 1,
    Multiplier
};

enum class AttributeType : uint8_t {
    Power = 1,
    Soul,
    Level
};

struct AttributeGain {
    Target target;
    AttributeType type;
    ValueType gainType;
    int32_t value;
    std::optional<Multiplier> modifier;
    int duration;
};

struct Look {
    Number number;
    Place place;
    ValueType valueType;
    std::optional<Multiplier> multiplier;
};

enum class RevealType : uint8_t {
    TopDeck = 1,
    ChosenCards = 2,
    FromHand = 3
};

struct RevealCard {
    RevealType type;
    Number number;
    std::optional<Card> card;
};

enum class Order : uint8_t {
    NotSpecified = 0,
    Any,
    Same
};

struct MoveCard {
    Player executor;
    Target target;
    Place from;
    std::vector<Place> to;
    Order order;
};

struct PayCost {
    std::vector<Effect> ifYouDo;
    std::vector<Effect> ifYouDont;
};

struct AbilityGain {
    Target target;
    int number;
    std::vector<Ability> abilities;
    int duration;
};

struct PerformEffect {
    int numberOfEffects;
    int numberOfTimes;
    std::vector<EventAbility> effects;
};

struct SearchTarget {
    Number number;
    std::vector<Card> cards;
};

struct SearchCard {
    std::vector<SearchTarget> targets;
    Place place;
};

struct MoveWrToDeck {
    Player executor;
};

struct FlipOver {
    Number number;
    Card forEach;
    std::vector<Effect> effect;
};

struct Backup {
    int32_t power;
    int8_t level;
};

struct NonMandatory {
    std::vector<Effect> effect;
    std::vector<Effect> ifYouDo;
    std::vector<Effect> ifYouDont;
};

struct ChangeState {
    Target target;
    State state;
};

struct DealDamage {
    ValueType damageType;
    int damage;
    std::optional<Multiplier> modifier;
};

enum class BackupOrEvent : uint8_t {
    Backup = 1,
    Event,
    Both
};

struct CannotUseBackupOrEvent {
    BackupOrEvent what;
    Player player;
    int duration;
};

struct SwapCards {
    ChooseCard first;
    ChooseCard second;
};

enum class AttackType : uint8_t {
    Any = 0,
    FronalAttack,
    SideAttack
};

struct CannotAttack {
    Target target;
    AttackType type;
    int duration;
};

struct AddMarker {
    Target target;
    Target destination;
    FaceOrientation orientation;
};

struct Bond {
    std::string value;
};

struct PerformReplay {
    std::string value;
};

struct Replay {
    std::string name;
    std::vector<Effect> effects;
};

struct DrawCard {
    Player executor;
    Number value;
};

struct Shuffle {
    Zone zone;
    Player owner;
};

struct CannotBecomeReversed {
    Target target;
    int duration;
};

struct OpponentAutoCannotDealDamage {
    int duration;
};

struct CannotMove {
    Target target;
    int duration;
};

struct SideAttackWithoutPenalty {
    Target target;
    int duration;
};

struct PutOnStageRested {
    Target target;
    Place from;
    Position to;
};

struct RemoveMarker {
    Target targetMarker;
    Target markerBearer;
    Place place;
};

struct OtherEffect {
    std::string cardCode;
    int effectId;
};


enum class EffectType : uint8_t {
    NotSpecified = 0,
    AttributeGain = 1,
    ChooseCard,
    RevealCard,
    MoveCard,
    SearchCard,
    PayCost,
    AbilityGain,
    MoveWrToDeck,
    FlipOver,
    Backup,
    TriggerCheckTwice,
    Look,
    NonMandatory,
    EarlyPlay,
    CannotPlay,
    PerformEffect,
    ChangeState,
    DealDamage,
    CannotUseBackupOrEvent,
    DrawCard,
    SwapCards,
    CannotAttack,
    CharAutoCannotDealDamage,
    OpponentAutoCannotDealDamage,
    CannotBecomeReversed,
    StockSwap,
    AddMarker,
    Bond,
    CannotMove,
    PerformReplay,
    Replay,
    SideAttackWithoutPenalty,
    Standby,
    Shuffle,
    PutOnStageRested,
    RemoveMarker,

    OtherEffect = 255
};

struct Effect {
    EffectType type;
    Condition cond;
    std::variant<
        std::monostate,
        AttributeGain,
        ChooseCard,
        RevealCard,
        MoveCard,
        SearchCard,
        PayCost,
        AbilityGain,
        PerformEffect,
        MoveWrToDeck,
        FlipOver,
        Backup,
        Look,
        NonMandatory,
        ChangeState,
        DealDamage,
        CannotUseBackupOrEvent,
        SwapCards,
        CannotAttack,
        AddMarker,
        Bond,
        PerformReplay,
        Replay,
        DrawCard,
        Shuffle,
        CannotBecomeReversed,
        OpponentAutoCannotDealDamage,
        CannotMove,
        SideAttackWithoutPenalty,
        PutOnStageRested,
        RemoveMarker,
        OtherEffect
    > effect;
};

}
