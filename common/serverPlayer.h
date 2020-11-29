#pragma once

class ServerGame;
class ServerProtocolHandler;

class ServerPlayer
{
    ServerGame *mGame;
    ServerProtocolHandler *mClient;
    size_t mId;
public:
    ServerPlayer(ServerGame *game, ServerProtocolHandler *client, size_t id);

    size_t id() { return mId; }
};
