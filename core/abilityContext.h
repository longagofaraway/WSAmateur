#pragma once

#include <vector>

#include "cardImprint.h"

struct AbilityContext {
    std::vector<CardImprint> chosenCards;
    std::vector<CardImprint> lastMovedCards;
};
