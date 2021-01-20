#pragma once

#include "serverCardZone.h"
#include "serverPlayer.h"

class ServerStage : public ServerCardZone {
public:
    ServerStage(ServerPlayer *player);

    ServerCard* addCard(std::unique_ptr<ServerCard> card) override;
    std::unique_ptr<ServerCard> takeCard(int index) override;
    std::unique_ptr<ServerCard> putOnStage(std::unique_ptr<ServerCard> card, int pos) override;
    void switchPositions(int pos1, int pos2) override;
};

