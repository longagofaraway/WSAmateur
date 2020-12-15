#include "serverCard.h"

ServerCard::ServerCard(std::shared_ptr<CardInfo> info, ServerCardZone *zone)
    : mCardInfo(info), mZone(zone) {
    mCode = info->code();
    mPower = info->power();
    mSoul = info->soul();
}
