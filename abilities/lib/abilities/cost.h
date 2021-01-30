#pragma once

#include <variant>
#include <vector>

#include "effect.h"

namespace asn {


enum class CostType : uint8_t {
    Stock = 1,
    Effects
};

struct StockCost{
    int value;
};

struct CostItem {
    CostType type;
    std::variant<StockCost, Effect> costItem;
};


struct Cost {
    std::vector<CostItem> items;
};

}
