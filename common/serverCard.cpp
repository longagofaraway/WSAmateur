#include "serverCard.h"

ServerCard::ServerCard(std::shared_ptr<CardInfo> info, ServerCardZone *zone)
    : mCardInfo(info), mZone(zone) {
    mCode = info->code();
    mPower = info->power();
    mSoul = info->soul();
}

ServerCard::ServerCard(int pos, ServerCardZone *zone)
    : mZone(zone) {
    setPos(pos);
}

void ServerCard::setPos(int pos) {
    mPosition = pos;
    if (pos < 3)
        mRow = CenterStage;
    else
        mRow = BackStage;
}
