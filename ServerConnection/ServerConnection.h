#pragma once
#include <functional>
#include <enet/enet.h>
#include <string>
#include "Utils.h"
#include <vector>

struct GameStartInfo
{
    // The port you should use for the connection
    uint16_t port;

    // The list of candidate peerAddresses to try and connect to
    std::vector<ENetAddress> peerAddresses;
    
    // Which player you are - 1 or 2;
    int playerNumber;

    std::string peerName;
    std::string ToString() const;
};

struct GameInfoStruct
{
    GameInfoStruct(const std::string& msg);
    std::string ToString() const;
    std::string owner;
    std::vector<std::string> joined;
    std::vector<std::string> requested;
};

struct PlayerInfo
{
    std::string name;
    std::vector<char> data;
};
struct ServerCallbacks
{
    std::function<void()> Timeout;
    std::function<void()> Disconnected;    
    std::function<void()> Connected;

    /// You have successfully created a game and are waiting for players to join
    std::function<void()> GameCreatedOK;

    /// You have successfully left a game 
    std::function<void()> LeftGameOK;

    /// You have been removed from game, either because the creator left, disconnected, or kicked you off
    std::function<void()> RemovedFromGame;

    // A list of current users connected. Every time this changes, this event is called
    std::function<void(const std::vector<PlayerInfo>&)> UserList;

    // A list of current open games. Every time this change, this event is called
    std::function<void(const std::vector<std::string>&)> OpenGames;

    // Information about the current game you are involved in, either as host or joiner
    // Who hosted the game, who has joined, and who is pending
    std::function<void(const GameInfoStruct&)> GameInfo;

    // You are hosting a game and a player has asked to join
    std::function<void(const std::string&)> JoinRequestFromOtherPlayer;

    // Your request to join a game has been successful, now wait for the host to say yes or no
    std::function<void()> JoinRequestOK;

    // Your request to join a game has been successful, now wait for the host to say yes or no
    std::function<void(const std::string&)> Approved;

    // You are ready to start a P2P session
    std::function<void(const GameStartInfo&)> StartP2P;

    // You are hosting a game and a player has asked to join
    std::function<void(const std::string&)> ServerMessage;

};


enum class ServerConnectionState;
class ServerConnection
{
    // Can't copy or move
    ServerConnection(const ServerConnection&) = delete;
    ServerConnection& operator=(const ServerConnection&) = delete;
    ServerConnection(const ServerConnection&&) = delete;
    ServerConnection& operator=(const ServerConnection&&) = delete;
public:
    // Start Idle
    ServerConnection(std::function<void(const std::string&)> logger);

    // Start Connection immediately
    ServerConnection(const std::string& serverIP, int serverPort, const std::string& userName, const void* userData, size_t userDataLength,const std::string& gameID, std::function<void(const std::string&)> logger);
    ~ServerConnection();
    // Call regularly (ie every frame)
    void Update(ServerCallbacks& callbacks);
    
    // Returns false if already connected or connecting.
    bool Connect(const std::string& serverIP, int serverPort, const std::string& userName, const void* userData,size_t userDataLength, const std::string& gameID);
    bool Disconnect();
    bool RequestToJoinGame(const std::string& gameOwner) const;
    bool LeaveGame() const;
    bool CreateGame() const;
    bool StartGame() const;
    bool ApproveJoinRequest(const std::string& player);
    bool EjectPlayer(const std::string& player);
   // bool IsConnected() const;

private:
    std::vector<ENetAddress> m_localAddresses;
    ENetHostPtr m_local;
    ENetAddress m_serverAddress{ 0,0 };
    ENetPeer* m_server=nullptr;
    std::vector<uint32_t> ReturnLocalIPv4() const;
    std::string m_userName;
    std::string m_gameID;  
    std::vector<char> m_userData;
    ServerConnectionState m_state;
    std::unique_ptr < GameStartInfo > m_startGameInfo = nullptr;
    std::function<void(const std::string&)> m_logger;
};