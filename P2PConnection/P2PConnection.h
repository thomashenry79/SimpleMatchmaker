#pragma once
#include <functional>
#include <enet/enet.h>
#include <string>
#include "Utils.h"
#include <vector>
#include <string>
#include "ServerConnection.h"
class P2PConnection
{
public:
    P2PConnection(GameStartInfo info);
    ~P2PConnection()
    {
    }
    
    void Update();
private:
    GameStartInfo m_info;
    ENetAddress localAddress{ 0,0 };
    ENetHostPtr local;
    std::vector<ENetPeer*> outGoingPeerCandidates;
    std::vector<ENetPeer*> peerConnections;
};