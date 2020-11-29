#include "serverPlayer.h"

ServerPlayer::ServerPlayer(ServerGame *game, ServerProtocolHandler *client, size_t id)
    : mGame(game), mClient(client), mId(id) { }
