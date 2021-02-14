#include "print.h"

using namespace asn;

std::string printCostItem(const CostItem &c) {
    std::string s;

    if (c.type == CostType::Stock) {
        s += '(' + std::to_string(std::get<StockCost>(c.costItem).value) + ") ";
    } else {
        s += printEffect(std::get<Effect>(c.costItem));
    }

    return s;
}

std::string printCost(const Cost &c) {
    std::string s = "[";
    bool firstEffect = true;

    for (size_t i = 0; i < c.items.size(); ++i) {
        auto tmp = printCostItem(c.items[i]);
        if (firstEffect && c.items[i].type != CostType::Stock)
            tmp[0] = std::toupper(tmp[0]);
        s += tmp;
        // if not the last costItem, add comma
        if (i < c.items.size() - 1) {
            if (c.items[i].type != CostType::Stock && s[s.size() - 1] == ' ') {
                s[s.size() - 1] = ',';
                s.push_back(' ');
            }
        }
    }

    if (s[s.size() - 1] == ' ')
        s.pop_back();

    s += "] ";

    return s;
}
