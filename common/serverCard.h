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
    std::shared_ptr<CardInfo> mCardInfo;
    ServerCardZone *mZone;
    std::string mCode;

    // stage position
    StageRow mRow;
    size_t mPosition;

    int mPower;
    int mSoul;

public:
    ServerCard(std::shared_ptr<CardInfo> info, ServerCardZone *zone = nullptr);

    void setZone(ServerCardZone *zone) { mZone = zone; }
    const std::string& code() { return mCode; }
};
