#pragma once
#include <functional>
#include <enet/enet.h>
#include <string>
#include "Utils.h"
#include <vector>
#include <string>
#include "ServerConnection.h"
#include <chrono>

struct GGPOStartInfo
{
      int yourPlayerNumber; // 1 or 2;

    uint16_t yourPort;
    uint16_t opponentPort;
    std::string opponentIP;
};

struct P2PCallbacks
{
    std::function<void()> Connected;
    std::function<void()> Disconncted;
    std::function<void()> Timeout;
    std::function<void()> ReadyStatusChanged; // true - ready, false - not ready
    std::function<void(const GGPOStartInfo&)> StartGame; // Ready to start game
    std::function<void(const void*,size_t)> ReceiveUserMessage;
};

class PingHandler
{
public:
    PingHandler();
    void Update(ENetPeer* peerToPing);
    void OnPong();
    double GetPing() const;
private:
    // Exponential moving average, taken three a second, over the last 20 seconds
    double m_pingEMA = 0;
    double m_pingMean = 0;
    const int emaPeriodMS = 10 * 1000;
    const int pingPeriodMS = 250;
    const int m_nSamples = emaPeriodMS / pingPeriodMS;
    const double EMA_Constant = 2 / (1.0 + m_nSamples);
    unsigned int m_bPingSent =false;
    std::chrono::steady_clock::time_point lastPing;
    std::vector<double> m_pings;
};


class P2PConnection
{
    // Can't copy or move
    P2PConnection(const P2PConnection&) = delete;
    P2PConnection& operator=(const P2PConnection&) = delete;
    P2PConnection(const P2PConnection&&) = delete;
    P2PConnection& operator=(const P2PConnection&&) = delete;
public:
    P2PConnection(GameStartInfo info,std::function<void(const std::string&)> logger);
    ~P2PConnection();    
    void ToggleReady();
    void Update(P2PCallbacks& callbacks);    
    void TryStart();
    double GetPing() const;
    void SendUserMessage(char* buffer, size_t length);
    bool ImReady()const { return m_bMeReady; }
    bool OtherReady() const { return m_bOtherReady; }
private:
    const GameStartInfo m_info;
    const ENetAddress localAddress;
    ENetHostPtr local;
    std::vector<ENetAddress> peerCandidateAddresses;
    std::vector<ENetPeer*> outGoingPeerCandidates;
    std::vector<ENetPeer*> peerConnections;
    bool m_bMeReady=false;
    bool m_bOtherReady=false;
    bool m_TryStart = false;
    bool m_Start = false;
    bool CanSend()const;
    void OnReadyChange();
    bool ReadyToStart() const;
     void Info();
    ENetAddress m_PrimaryConnection;
    bool m_bPrimaryConnectionEstablished = false;
    size_t TotalActivePeers() const {
        return outGoingPeerCandidates.size() + peerConnections.size();
    }
    std::function<void(std::string)> m_logger;
    void CleanRedundantConnections();
    PingHandler m_pingHandler;
    enum class P2PState { Connecting, Connected, Ready};
};