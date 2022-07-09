#pragma once
#include <enet/enet.h>
#include <map>
#include <memory>
#include <string>
#include "User.h"
class User;
class Message;
class Connections
{
public:
    void NewConnection(ENetPeer* peer);
    void LostConnection(ENetPeer* peer);
    void ReceiveMessage(ENetPeer* peer, const unsigned char* data, size_t len);
    void Update();
    bool VerifyName(const std::string& name);
    bool VerifyVersion(const std::string& version);
private:
    std::map<ENetPeer*, std::unique_ptr<User>> users;
};