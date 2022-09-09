#pragma once
#include <functional>
#include <enet/enet.h>
#include <string>
#include "Utils.h"
#include <vector>
#include <string>
#include "ServerConnection.h"
#include <chrono>

struct P2PCallbacks
{
    std::function<void()> Connected;
    std::function<void()> Disconncted;
    std::function<void()> PlayerReady;
    std::function<void()> StartGame;
    std::function<void(const void*,size_t)> ReceiveMessage;
};

class PingHandler
{
public:
    PingHandler();
    void Update(ENetPeer* peerToPing);
    void OnPong();
    double GetPing() const { return m_pingEMA; }
private:
    // Exponential moving average, taken twice a second, over the last 15 seconds
    double m_pingEMA = 0;
    const int emaPeriodMS = 15 * 1000;
    const int pingPeriodMS = 500;
    const double EMA_Constant = 2 / (1.0 + (emaPeriodMS / pingPeriodMS));
    unsigned int m_bPingSent =false;
    std::chrono::steady_clock::time_point lastPing;
};

class P2PConnection
{
public:
    P2PConnection(GameStartInfo info,std::function<void(const std::string&)> logger);
    ~P2PConnection();    
    void SendReady();
    void Update(P2PCallbacks& callbacks);
    bool ReadyToStart() const;
    void Info();
    void SendStart();
    double GetPing() const;
    void SendUserMessage(char* buffer, size_t length);
private:
    GameStartInfo m_info;
    ENetAddress localAddress{ 0,0 };
    ENetHostPtr local;
    std::vector<ENetPeer*> outGoingPeerCandidates;
    std::vector<ENetPeer*> peerConnections;
    bool m_bMeReady=false;
    bool m_bOtherReady=false;
    bool m_Start = false;
    void OnReadyChange();
    
    bool m_bPrimaryConnectionEstablished = false;
    size_t TotalActivePeers() const {
        return outGoingPeerCandidates.size() + peerConnections.size();
    }
    std::function<void(std::string)> m_logger;
    void CleanRedundantConnections();
    PingHandler m_pingHandler;
};