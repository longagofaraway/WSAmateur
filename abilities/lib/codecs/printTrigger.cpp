#include "print.h"

#include <string>

std::string printZoneChangeTrigger(const ZoneChangeTrigger &t) {
    std::string s;

    if (t.target[0].type == TargetType::ThisCard)
        s += "When this card is placed ";
    if (t.to == AsnZone::Stage)
        s += "on stage ";
    if (t.from == AsnZone::Hand)
        s += "from your hand, ";

    return s;
}

std::string printTrigger(const Trigger &t) {
    std::string s;

    switch (t.type) {
    case TriggerType::OnZoneChange:
        s += printZoneChangeTrigger(std::get<ZoneChangeTrigger>(t.trigger));
        break;
    }

    return "";
}