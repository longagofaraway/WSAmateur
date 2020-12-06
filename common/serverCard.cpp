#include "serverCard.h"

ServerCard::ServerCard(const CardInfo &info, ServerCardZone *zone)
    : mCardInfo(info), mZone(zone) {
    mCode = info.code();
    mPower = info.power();
    mSoul = info.soul();
}
