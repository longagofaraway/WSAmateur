#pragma once

#include <optional>

#include <QString>

#include "abilities.h"

decltype(asn::Condition::cond) getDefaultCondition(asn::ConditionType type);

asn::Condition getConditionFromPreset(QString preset);
