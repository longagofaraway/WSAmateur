#pragma once

#include "abilities.h"

class ServerCard;

class AttributeChange {
public:
    asn::AttributeType attr;
    int value;
    int duration;

    AttributeChange(asn::AttributeType attr, int delta, int duration)
        : attr(attr), value(delta), duration(duration) {}
};

class ContAttributeChange {
public:
    // card, that gives this buff
    ServerCard *source;
    // ability of source that gives the buff
    int abilityId;
    asn::AttributeType attr;
    int value;
    bool positional;

    ContAttributeChange(ServerCard *card, int abilityId, asn::AttributeType attr, int value, bool positional = false)
        : source(card), abilityId(abilityId), attr(attr), value(value), positional(positional) {}
};

inline bool operator==(const ContAttributeChange &lhs, const ContAttributeChange &rhs) {
    return lhs.source == rhs.source && lhs.abilityId == rhs.abilityId &&
        lhs.attr == rhs.attr;
}

class AbilityBuff {
public:
    // card, that gives this buff
    ServerCard *source = nullptr;
    // ability of source that gives the buff
    int sourceAbilityId = 0;
    // ability given by source
    int abilityId = 0;
    bool positional = false;
    int duration = 0;

    // constructor for effects with duration
    AbilityBuff(int abilityId, int duration)
        : abilityId(abilityId), duration(duration) {}
    // constructor for cont effect
    AbilityBuff(ServerCard *card, int sourceAbilityId, bool positional)
        : source(card), sourceAbilityId(sourceAbilityId), positional(positional) {}
};

inline bool operator==(const AbilityBuff &lhs, const AbilityBuff &rhs) {
    return lhs.source == rhs.source && lhs.sourceAbilityId == rhs.sourceAbilityId;
}

enum class BoolAttributeType {
    CannotFrontAttack,
    CannotSideAttack,
    CannotBecomeReversed,
    CannotMove,
    SideAttackWithoutPenalty,
    CannotStand,
    CannotBeChosen
};

class BoolAttributeChange {
public:
    // card, that gives this buff
    ServerCard *source = nullptr;
    // ability of source that gives the buff
    int abilityId = 0;
    BoolAttributeType type;
    bool positional = false;
    int duration = 0;

    // constructor for effects with duration
    BoolAttributeChange(BoolAttributeType type, int duration)
        : type(type), duration(duration) {}
    // constructor for cont effect
    BoolAttributeChange(ServerCard *card, int abilityId, BoolAttributeType type, bool positional = false)
        : source(card), abilityId(abilityId), type(type), positional(positional) {}
};

inline bool operator==(const BoolAttributeChange &lhs, const BoolAttributeChange &rhs) {
    return lhs.source == rhs.source && lhs.abilityId == rhs.abilityId;
}



enum class BuffType {
    TriggerIcon = 0
};

class Buff {
public:
    // card, that gives this buff
    ServerCard *source = nullptr;
    // ability of source that gives the buff
    int abilityId = 0;
    bool positional = false;
    int duration = 0;
    BuffType type;

    Buff() = default;
    Buff(BuffType type, int duration)
        : duration(duration), type(type) {}
    Buff(ServerCard *source, int abilityId, BuffType type)
        : source(source), abilityId(abilityId), type(type) {}

    virtual ~Buff() = default;
    virtual void apply(ServerCard *card) const = 0;
    virtual void update(const Buff &buff) const = 0;
    virtual void undo(ServerCard *card) const = 0;
    virtual std::unique_ptr<Buff> clone() const = 0;

protected:
    void fillBaseFields(Buff *newBuff, const Buff *sourceBuff) const;
};

inline bool operator==(const Buff &lhs, const Buff &rhs) {
    return lhs.type == rhs.type && lhs.source == rhs.source &&
        lhs.abilityId == rhs.abilityId;
}
inline bool operator==(const std::unique_ptr<Buff> &lhs, const Buff &rhs) {
    return lhs->type == rhs.type && lhs->source == rhs.source &&
        lhs->abilityId == rhs.abilityId;
}

class TriggerIconBuff : public Buff {
public:
    asn::TriggerIcon icon;

    TriggerIconBuff() = default;
    TriggerIconBuff(asn::TriggerIcon icon, int duration)
        : Buff(BuffType::TriggerIcon, duration), icon(icon) {}
    TriggerIconBuff(asn::TriggerIcon icon, ServerCard *source, int abilityId)
        : Buff(source, abilityId, BuffType::TriggerIcon), icon(icon) {}
    void apply(ServerCard *card) const override;
    void update(const Buff &buff) const override {}
    void undo(ServerCard *card) const override;
    std::unique_ptr<Buff> clone() const override;
};
