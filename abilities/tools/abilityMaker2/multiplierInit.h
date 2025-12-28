#pragma once

#include <optional>

#include <QString>

#include "abilities.h"

decltype(asn::Multiplier::specifier) getDefaultMultiplier(asn::MultiplierType type);

//std::optional<asn::Multiplier> getMultiplierFromPreset(QString preset);
