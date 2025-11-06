#pragma once

#include <optional>

#include <QString>

#include "abilities.h"

decltype(asn::Trigger::trigger) getDefaultTrigger(asn::TriggerType type);

std::optional<asn::Trigger> getTriggerFromPreset(QString preset);
