#include <stdint.h>
#include "ServerConnection.h"
#include "Utils.h"
#include "Message.h"
#include "Sender.h"
#include <ws2tcpip.h>
#include <algorithm>

GameInfoStruct::GameInfoStruct(const std::string& msg)
{
    auto parts = stringSplit(msg, ':');
    owner = parts[0];
    if(parts[1].length())
        joined = stringSplit(parts[1], ',');
    if (parts.size() >2 && parts[2].length())
        requested =  stringSplit(parts[2], ',');
}


std::string GameInfoStruct::ToString() const
{
    std::string info;
    info += "Created by: " + owner + ".";
    info += "Joined: ";
    for (const auto& p : joined)
        info += p + ",";
   
    // remove the trailing "," if neeeded
    if (joined.size())
        info.pop_back();
    else
        info+= "None";

    info += ". Pending: ";

    for (const auto& p : requested)
        info += p + ",";

    if (requested.size())
        info.pop_back();
    else
        info += "None";
    return info;
}

enum class ServerConnectionState
{
    Idle,
    Connecting,   
    Connected
};

ServerConnection::~ServerConnection()
{
    Disconnect();    
}


std::vector<uint32_t> ServerConnection::ReturnLocalIPv4() const{
    char host[256];
   // char* IP;
    struct hostent* host_entry;
    std::vector<uint32_t> addresses;
    int hostname;
    int i = 0;
    hostname = gethostname(host, sizeof(host)); //find the host name
    m_logger(std::string("Current Host Name: ") + host + "\n");
    host_entry = gethostbyname(host); //find host information

    if (host_entry->h_addrtype == AF_INET) {
        while (host_entry->h_addr_list[i] != 0) {
            auto address = *((struct in_addr*)host_entry->h_addr_list[i++]);
            printf("\tIPv4 Address #%d: %s\n", i, inet_ntoa(address));
            addresses.push_back(address.S_un.S_addr);
        }
    }
    else if (host_entry->h_addrtype == AF_INET6)
        printf("\tRemotehost is an IPv6 address\n");

   // auto address = *((struct in_addr*)host_entry->h_addr_list[0]);
  //  IP = inet_ntoa(addr); //Convert into IP string
   
    // printf("Internal IP: %s\n", IP);
    return addresses;
}



ServerConnection::ServerConnection(std::function<void(const std::string&)> logger) :
    m_state(ServerConnectionState::Idle),
    m_local(nullptr, [](ENetHost*) {}),
    m_logger(logger)
{
    
}

ServerConnection::ServerConnection(
    const std::string& serverIP,
    int serverPort,
    const std::string& userName,
    const void* userData, size_t userDataLength,
    const std::string& gameID,
    std::function<void(const std::string&)> logger) :
    ServerConnection(logger)
{
    Connect(serverIP, serverPort, userName, userData,userDataLength,gameID);
}

bool ServerConnection::Connect(const std::string& serverIP, int serverPort, const std::string& userName,  const void* userData, size_t userDataLength, const std::string& gameID)
{ 
    if (m_state != ServerConnectionState::Idle)
        return false;
  
    m_startGameInfo = nullptr;
    m_userName = userName;
    auto asChar = (const char*)userData;
    m_userData.assign(asChar, asChar+ userDataLength);
    ::eraseAndRemove(m_userName, ',');
    ::eraseAndRemove(m_userName, ':');

    if (m_userName.length() == 0)
        return false;

    m_local = ENetHostPtr(enet_host_create(nullptr, 1, 0, 0, 0), enet_host_destroy);  

 

    enet_address_set_host_ip(&m_serverAddress, serverIP.c_str());
    m_serverAddress.port = serverPort;
    m_server = enet_host_connect(m_local.get(), &m_serverAddress, 0, 0);
    m_state = ServerConnectionState::Connecting;

   

m_gameID = gameID;
return true;
}

bool ServerConnection::RequestToJoinGame(const std::string& gameOwner) const
{
    if (m_server)
        Message::Make(MessageType::Join, gameOwner).OnData(SendTo(m_server));

    return true;
}
bool ServerConnection::ApproveJoinRequest(const std::string& player)
{
    if (m_server)
        Message::Make(MessageType::Approve, player).OnData(SendTo(m_server));
    return true;
}

bool ServerConnection::EjectPlayer(const std::string& player)
{
    if (m_server)
        Message::Make(MessageType::Eject, player).OnData(SendTo(m_server));
    return true;
}


bool ServerConnection::LeaveGame() const
{
    if (m_server)
        Message::Make(MessageType::Leave, "").OnData(SendTo(m_server));
    return true;
}

bool ServerConnection::CreateGame() const
{
    if (m_server)
        Message::Make(MessageType::Create, "2,2").OnData(SendTo(m_server));
    return true;
}

bool ServerConnection::StartGame() const
{
    if (m_server)
        Message::Make(MessageType::Start, "").OnData(SendTo(m_server));
    return true;
}
bool ServerConnection::Disconnect()
{
    if (m_state == ServerConnectionState::Connecting)
        return false;
    if (m_server)
        enet_peer_disconnect(m_server, 0);

    if (m_local)
        enet_host_flush(m_local.get());

    return true;
}
std::vector<PlayerInfo> ParsePlayerInfo(const void* data, size_t len)
{
    std::vector<PlayerInfo> info;
    std::string message((const char*)data, len);
    auto pos = message.find_first_of(':');
    auto numberOfPlayers = std::stoul(message.substr(0, pos++));

    for (auto i = 0u; i < numberOfPlayers; i++)
    {
        auto next = message.find_first_of(':', pos);
        auto name = message.substr(pos, next - pos);
        pos = next;
        next = message.find_first_of(':', ++pos);
        auto lengthOfUserData = std::stoul(message.substr(pos, next - pos));
        pos = ++next;
        next = pos + lengthOfUserData;
        std::vector<char> data((char*)data + pos, (char*)data + next);
        pos = next;
        info.push_back({ name,data });
    }
    return info;
}
void ServerConnection::Update(ServerCallbacks& callbacks)
{
    if (!m_local)
        return;
    ENetEvent event;
    while (enet_host_service(m_local.get(), &event, 1) > 0)
    {
        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT: {
            // std::cout << "connect event, ip:  " << ToReadableString(event.peer->address) << "\n";

            if (event.peer == m_server && m_serverAddress == event.peer->address) {

                m_state = ServerConnectionState::Connected;
                // Send the details needed to the server
                Message::Make(MessageType::Version, m_gameID).OnData(SendTo(m_server));
                auto localIPs = ReturnLocalIPv4();
                m_localAddresses.resize(localIPs.size());
                std::string ipMessage;
                for (size_t i = 0; i < localIPs.size(); i++)
                {
                    enet_socket_get_address(m_local->socket, &m_localAddresses[i]);
                    m_localAddresses[i].host = localIPs[i];
                    if (i > 0)
                        ipMessage += ",";
                    ipMessage += ToString(m_localAddresses[i]);
                }
              
                Message::Make(MessageType::Info, ipMessage).OnData(SendTo(m_server));
                Message::Make(MessageType::Login, m_userName+":"+std::string(m_userData.data(),m_userData.size())).OnData(SendTo(m_server));
            }
            else
            {
                enet_peer_reset(event.peer);
            }
            break;
        }
        case ENET_EVENT_TYPE_RECEIVE:
        {
            EnetPacketRAIIGuard guard(event.packet);
            auto msg = Message::Parse(event.packet->data, event.packet->dataLength);
    
           
            if (msg.Type() == MessageType::Login)
            {               
                callbacks.Connected();               
            }

            if (msg.Type() == MessageType::Create)
            {
                callbacks.GameCreatedOK();
            }
            
            if (msg.Type() == MessageType::PlayersActive)
            {
                msg.OnPayload([&](const void* data, size_t len) {
                    auto info = ParsePlayerInfo(data, len);
                    callbacks.UserList(info);
                });
            }
            
            if (msg.Type() == MessageType::Join)
            {
                auto name = msg.Content();

                if (name == m_userName)
                    callbacks.JoinRequestOK();
                else
                    callbacks.JoinRequestFromOtherPlayer(name);
            }
            
            if (msg.Type() == MessageType::GamesOpen)
            {
                auto games = stringSplit(msg.Content(), ',');
                callbacks.OpenGames(games);
            }
            
            if(msg.Type() == MessageType::GameInfo)
            { 
                callbacks.GameInfo(GameInfoStruct(msg.Content()));
            }
            
            if(msg.Type() == MessageType::Leave)
            { 
                callbacks.LeftGameOK();
            }

            if (msg.Type() == MessageType::Eject)
            {
                callbacks.RemovedFromGame();
            }

            if (msg.Type() == MessageType::Approve)
            {                
                callbacks.Approved(msg.Content());
                // For now, as soon as a player is approved, we start the game. 
                // Later, we will allow the user to trigger this when enough players have joined the room
                StartGame();
            }
            if (msg.Type() == MessageType::Start)
            {
                m_startGameInfo.reset( new GameStartInfo);
                auto parts = stringSplit(msg.Content(),',');
                m_startGameInfo->playerNumber = std::stoi(parts[0]);
                m_startGameInfo->peerName = parts[1];
                ENetAddress temp;
                enet_socket_get_address(m_local->socket, &temp);
                m_startGameInfo->port = temp.port;

                ENetAddress addr;
                for (size_t i = 2; i < parts.size(); i++)
                {
                    if (TryParseIPAddress(parts[i], addr))
                    {
                        if(!contains(m_startGameInfo->peerAddresses,addr))
                            m_startGameInfo->peerAddresses.push_back(addr);
                    }
                }
                /*if (TryParseIPAddress(parts[2], addr))
                    m_startGameInfo->peerAddresses.push_back(addr);

                if (TryParseIPAddress(parts[3], addr))
                {
                    if(addr != m_startGameInfo->peerAddresses.back())
                        m_startGameInfo->peerAddresses.push_back(addr);
                }*/

                enet_peer_disconnect(m_server,0);
                break;
            }

            if (msg.Type() == MessageType::Info)
            {
                callbacks.ServerMessage(msg.Content());
            }

            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT:
        {
            m_logger(std::string("Disconnection event from peer ")+ ToReadableString(event.peer->address) + "\n");
              
            if (m_server == event.peer)
            {
                
                
                m_server = nullptr;
                m_local = nullptr;
                if (m_startGameInfo)
                {
                    callbacks.StartP2P(*m_startGameInfo);
                    m_startGameInfo = nullptr;
                }
                else if (m_state == ServerConnectionState::Connecting)
                {
                    // std::cout << "Timeout connecting to server: " << "\n";
                    callbacks.Timeout();
                }
                else
                {
                    callbacks.Disconnected();
                }
                m_state = ServerConnectionState::Idle;

                return;
               
            }
            break;
        }
        
        }
     
    }
}

std::string GameStartInfo::ToString() const
{
    auto returnString = std::string("You are player ") + std::to_string(playerNumber) + ", other player is " +  peerName + "\n";
    returnString += "Use port " + std::to_string(this->port) + ", " + "other player's IP addresses are: ";
    returnString += ToReadableString(this->peerAddresses[0]);
    if(peerAddresses.size()>1)
        returnString += " and " + ToReadableString(this->peerAddresses[1]);
    returnString += "\n";
    return returnString;
}