#pragma once

#include <vector>

#include "attributeChange.h"


class CardBuffManager {
    ServerCard *mCard;

    std::vector<AttributeChange> mBuffs;
    std::vector<ContAttributeChange> mContBuffs;
    std::vector<BoolAttributeChange> mBoolAttrChanges;
    std::vector<AbilityBuff> mAbilityBuffs;

public:
    CardBuffManager(ServerCard *card) : mCard(card) {}

    void reset();

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
    void removePositionalContAttrBuffs();
    void removePositionalContBoolAttrChanges();
    void removeAbilityAsPositionalContBuff();

    void removePositionalContBuffsBySource(ServerCard *source);
    void removePositionalContAttrBuffsBySource(ServerCard *source);
    void removeAbilityAsPositionalContBuffBySource(ServerCard *source);
    void removePositionalContBoolAttrChangeBySource(ServerCard *source);

    void endOfTurnEffectValidation();
    void validateAttrBuffs();
    void validateAbilities();
    void validateBoolAttrChanges();

private:
    int addAbility(const asn::Ability &a);
    bool hasBoolAttrChange(BoolAttributeType type) const;
};
