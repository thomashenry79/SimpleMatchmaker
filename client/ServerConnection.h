#pragma once
#include <functional>
#include <enet/enet.h>
#include <string>
#include "Utils.h"
#include <vector>
#include <string>
struct GameStartInfo
{
    int port;
    std::vector<ENetAddress> peerAddresses;
    int playerNumber;
};

struct GameInfoStruct
{
    GameInfoStruct(const std::string& msg);
    std::string ToString() const;
    std::string owner;
    std::vector<std::string> joined;
    std::vector<std::string> requested;
};

struct ServerCallbacks
{
    std::function<void()> Disconnected;    
    std::function<void()> Connected;
    std::function<void(const std::string&)> GameCreated;
    std::function<void(const std::vector<std::string>&)> UserList;
    std::function<void(const std::vector<std::string>&)> OpenGames;
    std::function<void(const GameInfoStruct&)> GameInfo;
    std::function<void(const std::string&)> JoinRequest;
    std::function<void()> UserLeft;
};


enum class ServerConnectionState;
class ServerConnection
{
public:
    // Start Idle
    ServerConnection();

    // Start Connection immediately
    ServerConnection(const std::string& serverIP, int serverPort, const std::string& userName, const std::string& gameID);
    void Update(ServerCallbacks& callbacks);
    
    // Returns false if the already connected or connecting.
    bool Connect(const std::string& serverIP, int serverPort, const std::string& userName, const std::string& gameID);
    bool Disconnect();
    bool RequestToJoinGame(const std::string& gameOwner) const;
    bool LeaveGame() const;
    bool CreateGame() const;
    bool ApproveJoinRequest(const std::string& player);
    bool EjectPlayer(const std::string& player);
    bool IsConnected() const;

private:
    ENetAddress m_localAddress{ 0,0 };
    ENetHostPtr m_local;
    ENetAddress m_serverAddress{ 0,0 };
    ENetPeer* m_server=nullptr;

    std::string m_userName;
    std::string m_gameID;  
    ServerConnectionState m_state;
};