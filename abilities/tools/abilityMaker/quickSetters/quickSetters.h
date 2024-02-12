#pragma once

#include <QString>

#include "abilities.h"

asn::Trigger createTrigger(QString triggerName);
void addCostItem(asn::Cost &cost, QString costName);
