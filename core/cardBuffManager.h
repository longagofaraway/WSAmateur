#pragma once

#include <vector>

#include "attributeChange.h"


class CardBuffManager {
    using CardBuffs = std::vector<std::unique_ptr<Buff>>;

    ServerCard *mCard;

    std::vector<AttributeChange> mBuffs;
    std::vector<ContAttributeChange> mContBuffs;
    std::vector<BoolAttributeChange> mBoolAttrChanges;
    std::vector<AbilityBuff> mAbilityBuffs;

    CardBuffs cardBuffs;

    // we are assuming you can remove trait until end of turn
    // and give a trait during same turn (and it stays)
    // so we remember the order in which trait changes were applied
    std::vector<const TraitChange*> appliedTraitChanges;

public:
    CardBuffManager(ServerCard *card) : mCard(card) {}

    void reset();

    // new api
    void addBuff(const Buff &buff);
    void removeContBuff(const Buff &buff);

    // old api
    void sendAttrChange(asn::AttributeType attr);
    void sendChangedAttrs(std::tuple<int, int, int> oldAttrs);
    void sendBoolAttrChange(BoolAttributeType type, bool value);

    void addAttributeBuff(asn::AttributeType attr, int delta, int duration = 1);
    void addBoolAttrChange(BoolAttributeType type, int duration);
    void addContAttributeBuff(ServerCard *source, int abilityId, asn::AttributeType attr, int delta, bool positional = false);
    void addContBoolAttrChange(ServerCard *source, int abilityId, BoolAttributeType type, bool positional);

    void removeContAttributeBuff(ServerCard *source, int abilityId, asn::AttributeType attr);
    void removeContBoolAttrChange(ServerCard *source, int abilityId, BoolAttributeType type);

    void addAbilityBuff(const asn::Ability &a, int duration);
    void addAbilityAsContBuff(ServerCard *source, int sourceAbilityId, const asn::Ability &ability, bool positional);
    void removeAbility(int abilityId);
    void removeAbilityAsContBuff(ServerCard *source, int sourceAbilityId);

    void removePositionalContBuffs();

    void removePositionalContBuffsBySource(ServerCard *source);

    void endOfTurnEffectValidation();
    void validateCannotStand();

    bool hasBoolAttrChangeEx(BoolAttributeType type) const;

    void addTraitChange(const TraitChange *traitChange);
    void removeTraitChange(const TraitChange *traitChange);

    const std::vector<const TraitChange*>& traitChanges() const { return appliedTraitChanges; }

private:
    CardBuffs::iterator removeBuff(CardBuffs::iterator it);

    bool shouldSkipEffectValidation(const Buff &buff);

    int addAbility(const asn::Ability &a);
    bool hasBoolAttrChange(BoolAttributeType type) const;

    void removePositionalContAttrBuffs();
    void removePositionalContBoolAttrChanges();
    void removeAbilityAsPositionalContBuff();

    void removePositionalContAttrBuffsBySource(ServerCard *source);
    void removeAbilityAsPositionalContBuffBySource(ServerCard *source);
    void removePositionalContBoolAttrChangeBySource(ServerCard *source);

    void validateAttrBuffs();
    void validateAbilities();
    void validateBoolAttrChanges();
};
