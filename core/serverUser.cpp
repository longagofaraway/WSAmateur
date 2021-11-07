#include "serverUser.h"

ServerUser::ServerUser(ServerProtocolHandler *client, int id)
    : client(client), _id(id) {}
