#pragma once
#include <functional>
#include <enet/enet.h>
#include <string>
#include "Utils.h"
#include <vector>
#include <string>
#include "ServerConnection.h"
#include <chrono>
class P2PConnection
{
public:
    P2PConnection(GameStartInfo info,std::function<void(const std::string&)> logger);
    ~P2PConnection();
    void SendPing();
    void SendReady();
    void Update();
    bool ReadyToStart() const;
    void Info();
private:
    GameStartInfo m_info;
    ENetAddress localAddress{ 0,0 };
    ENetHostPtr local;
    std::vector<ENetPeer*> outGoingPeerCandidates;
    std::vector<ENetPeer*> peerConnections;
    std::chrono::steady_clock::time_point lastPing;
    bool m_bMeReady=false;
    bool m_bOtherReady=false;
    bool m_Start = false;
    void OnReadyChange();
    void SendStart();
    bool m_bPrimaryConnectionEstablished = false;
    size_t TotalActivePeers() const {
        return outGoingPeerCandidates.size() + peerConnections.size();
    }
    std::function<void(std::string)> m_logger;
};