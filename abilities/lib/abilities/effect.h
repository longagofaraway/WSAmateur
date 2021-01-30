#pragma once

#include <variant>
#include <optional>

#include "condition.h"
#include "number.h"
#include "target.h"

namespace asn {

struct Ability;
struct Effect;

enum class PlaceType : uint8_t {
    Selection = 1,
    SpecificPlace
};

struct ChooseCard {
    std::vector<Target> targets;
    std::vector<Card> excluding;
    PlaceType placeType;
    std::optional<Place> place;
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
    std::vector<Effect> effects;
};

struct SearchTarget {
    Number number;
    std::vector<Card> cards;
};

struct SearchCard {
    std::vector<SearchTarget> targets;
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
};

struct SwapCards {
    ChooseCard first;
    ChooseCard second;
};

struct AddMarker {
    Target target;
    Target destination;
};

struct Bond {
    std::string name;
};

struct PerformReplay {
    std::string name;
};

struct Replay {
    std::string name;
    std::vector<Effect> effects;
};

struct DrawCard {
    Number value;
};

struct HardcodedEffect {
    std::string cardName;
};


enum class EffectType : uint8_t {
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
    CannotFrontAttack,
    CannotSideAttack,
    OpponentCharAutoCannotDealDamage,
    CannotBecomeReversed,
    StockSwap,
    AddMarker,
    Bond,
    CannotMove,
    PutRestedInSameSlot,
    PerformReplay,
    Replay,
    SideAttackWithoutPenalty,
    Standby,
    HardcodedEffects
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
        AddMarker,
        Bond,
        PerformReplay,
        Replay,
        DrawCard,
        HardcodedEffect
    > effect;
};

}
