#include "hardcodedEffects.h"

asn::ChooseCard createChooseCard(const asn::TargetAndPlace &target) {
    std::vector<asn::TargetAndPlace> v;
    v.push_back(target);
    return asn::ChooseCard {
        .executor = asn::Player::Player,
        .targets = v
    };
}
