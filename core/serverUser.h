#pragma once

#include <string>

class ServerProtocolHandler;

class ServerUser
{
    ServerProtocolHandler *client;
    int _id = 0;
    std::string _name;

public:
    ServerUser(ServerProtocolHandler *client, int id);

    int id() const { return _id; }
    const std::string& name() const { return _name; }
    void setName(const std::string &name_) { _name = name_; }
};

