#pragma once
#include "UserMap.h"
#include "UserChangedStateVisitor.h"
#include "IConnections.h"
#include <string>
#include "Utils.h"
class User;
class Message;


class Connections : public IConnections
{
public:
    Connections(uint16_t port);
    void NewConnection(ENetPeer* peer);
    void LostConnection(ENetPeer* peer);
    void ReceiveMessage(ENetPeer* peer, const unsigned char* data, size_t len);
    void Update();
    bool VerifyName(const std::string& name) override ;
    bool VerifyVersion(const std::string& version) override;
    UserChangedStateVisitor& UserChangeStateHandler() override { return visitor; }
    ENetHost* Host() { return m_host.get(); }
    void BroadcastMessage(const class Message& m) const override;
    void BroadcastActiveUsers() const override;
private:

    std::vector<User*> ActivePlayers() const;
    ENetAddress address;
    ENetHostPtr m_host;
    UserMap users;
    UserChangedStateVisitor visitor;   
};