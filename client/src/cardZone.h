#pragma once

#include <vector>

#include "card.h"

class QQuickItem;

class CardZone
{
public:
    virtual ~CardZone() {}

    virtual QQuickItem* visualItem() const = 0;
    virtual std::vector<Card>& cards() = 0;
    virtual void removeCard(int index) = 0;
};
