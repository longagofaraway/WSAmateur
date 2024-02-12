#include "quickSetters.h"


asn::Trigger createTrigger(QString triggerName) {
    if (triggerName == "Placed from hand to stage") {
        auto trigger = asn::ZoneChangeTrigger();
        trigger.from = asn::Zone::Hand;
        trigger.to = asn::Zone::Stage;
        auto target = asn::Target();
        target.type = asn::TargetType::ThisCard;
        trigger.target.push_back(target);

        asn::Trigger t;
        t.type = asn::TriggerType::OnZoneChange;
        t.trigger = trigger;
        return t;
    }
    if (triggerName == "When this card attacks") {
        auto trigger = asn::OnAttackTrigger();
        auto target = asn::Target();
        target.type = asn::TargetType::ThisCard;
        trigger.target = target;

        asn::Trigger t;
        t.type = asn::TriggerType::OnAttack;
        t.trigger = trigger;
        return t;
    }
    if (triggerName == "When this becomes reversed") {
        auto trigger = asn::StateChangeTrigger();
        auto target = asn::Target();
        target.type = asn::TargetType::ThisCard;
        trigger.target = target;
        trigger.state = asn::State::Reversed;

        asn::Trigger t;
        t.type = asn::TriggerType::OnStateChange;
        t.trigger = trigger;
        return t;
    }
    assert(false);
    return {};
}
