#pragma once

#include <string>

#include "cardInfo.h"

class ServerCardZone;

enum StageRow {
    CenterStage,
    BackStage
};

class ServerCard
{
    CardInfo mCardInfo;
    ServerCardZone *mZone;
    std::string mCode;

    // stage position
    StageRow mRow;
    size_t mPosition;

    int mPower;
    int mSoul;

public:
    ServerCard(const CardInfo &info, ServerCardZone *zone = nullptr);

    void setZone(ServerCardZone *zone) { mZone = zone; }
    const std::string& code() { return mCode; }
};
