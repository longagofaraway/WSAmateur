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
    CardState mState = CardState::Standing;

public:
    ServerCard(std::shared_ptr<CardInfo> info, ServerCardZone *zone);
    ServerCard(size_t pos, ServerCardZone *zone);

    void setPos(size_t pos);
    void setZone(ServerCardZone *zone) { mZone = zone; }
    const std::string& code() { return mCode; }
    int level() const { return mCardInfo->level(); }
    int cost() const { return mCardInfo->cost(); }
    CardType type() const { return mCardInfo->type(); }
    char color() const { return mCardInfo->color(); }
    CardState state() const { return mState; }
    void setState(CardState state) { mState = state; }
};
