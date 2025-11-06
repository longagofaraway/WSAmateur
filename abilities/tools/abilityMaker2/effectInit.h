#pragma once

#include <optional>

#include <QString>

#include "abilities.h"

decltype(asn::Effect::effect) getDefaultEffect(asn::EffectType type);

asn::Effect getEffectFromPreset(QString preset);
