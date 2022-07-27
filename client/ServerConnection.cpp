#include <stdint.h>
#include "ServerConnection.h"
#include "Utils.h"
#include "Message.h"
#include "Sender.h"
#include "Utils.h"
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
    Disconnecting,
    Connected,
    Hosting,
    Joining,
    Joined
};
bool isConnected(ServerConnectionState state)
{
    static std::vector<ServerConnectionState> nonConnectedStates{
        ServerConnectionState::Idle,
        ServerConnectionState::Connecting,
        ServerConnectionState::Disconnecting };

    return std::find(RANGE(nonConnectedStates), state) == std::end(nonConnectedStates);
}
bool ServerConnection::IsConnected() const
{
    return isConnected(m_state);
}

uint32_t ReturnLocalIPv4() {
    char host[256];
    char* IP;
    struct hostent* host_entry;
    int hostname;
    hostname = gethostname(host, sizeof(host)); //find the host name

    host_entry = gethostbyname(host); //find host information

    auto addr = *((struct in_addr*)host_entry->h_addr_list[0]);
    IP = inet_ntoa(addr); //Convert into IP string
    printf("Current Host Name: %s\n", host);
    // printf("Internal IP: %s\n", IP);
    return addr.S_un.S_addr;
}



ServerConnection::ServerConnection() :
    m_local(enet_host_create(nullptr, 1, 0, 0, 0), enet_host_destroy),
    m_state(ServerConnectionState::Idle)
{
    enet_socket_get_address(m_local->socket, &m_localAddress);
    m_localAddress.host = ReturnLocalIPv4();
}

ServerConnection::ServerConnection(const std::string& serverIP, int serverPort, const std::string& userName, const std::string& gameID) :
    ServerConnection()
{
    Connect(serverIP, serverPort, userName, gameID);
}

bool ServerConnection::Connect(const std::string& serverIP, int serverPort, const std::string& userName, const std::string& gameID)
{
    if (m_state != ServerConnectionState::Idle)
        return false;

    enet_address_set_host_ip(&m_serverAddress, serverIP.c_str());
    m_serverAddress.port = serverPort;
    m_server = enet_host_connect(m_local.get(), &m_serverAddress, 0, 0);
    m_state = ServerConnectionState::Connecting;

    m_userName = userName;
    ::eraseAndRemove(m_userName, ',');
    ::eraseAndRemove(m_userName, ':');
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
    if(m_server)
        Message::Make(MessageType::Create, "2,2").OnData(SendTo(m_server));
    return true;
}
bool ServerConnection::Disconnect()
{
    if (m_server)
    {
        m_state = ServerConnectionState::Disconnecting;
        enet_peer_disconnect(m_server, 0);
    }
    return true;
}

void ServerConnection::Update(ServerCallbacks& callbacks)
{
    ENetEvent event;
    while (enet_host_service(m_local.get(), &event, 1) > 0)
    {
        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:        {
            std::cout << "connect event, ip:  " << ToReadableString(event.peer->address) << "\n";

            if (event.peer == m_server && m_serverAddress == event.peer->address) {
                printf("We connected to the server\n");              

                // Send the details needed to the server
                Message::Make(MessageType::Version, m_gameID).OnData(SendTo(m_server));
                Message::Make(MessageType::Info, ToString(m_localAddress)).OnData(SendTo(m_server));
                Message::Make(MessageType::Login, m_userName).OnData(SendTo(m_server));
            }
            else
            {
                std::cout << "Connected to unknown peer...... bin it\n";
                enet_peer_disconnect(event.peer, 0);
            }
            break;
        }
        case ENET_EVENT_TYPE_RECEIVE:
        {
            EnetPacketRAIIGuard guard(event.packet);
            auto msg = Message::Parse(event.packet->data, event.packet->dataLength);
           // printf("We received a message: ");
           // msg.ToConsole();
           
            if (msg.Type() == MessageType::Login)
            {
                m_state = ServerConnectionState::Connected;
                callbacks.Connected();               
            }

            if (msg.Type() == MessageType::Create)
            {
                m_state = ServerConnectionState::Hosting;
                callbacks.GameCreated(msg.Content());
            }
            
            if (msg.Type() == MessageType::PlayersActive)
            {
                auto users = stringSplit(msg.Content(), ',');
                callbacks.UserList(users);
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
           
            if (msg.Type() == MessageType::Start)
            {
                enet_peer_reset(m_server);
                //if (TryParseIPAddressList(msg.Content(), client.peerAddresses))
                //{
                //    std::cout << "IPs of peers in message: ";
                //    for (const auto& peerAddress : client.peerAddresses)
                //        std::cout << ToReadableString(peerAddress) << "\n";


                //    // initiate conncetion to peer
                //    printf("Try to connect to peers: \n");
                //    for (auto& ad : client.peerAddresses)
                //    {
                //        client.peerCandidates.push_back(enet_host_connect(client.local.get(), &ad, 0, 0));
                //        std::cout << ToReadableString(ad) << "\n";
                //    }
                //    std::cout << "\n";
                //}
            }
            if (msg.Type() == MessageType::Info)
            {
                if (!strcmp(msg.Content(), "Ping"))
                    Message::Make(MessageType::Info, "Pong").OnData(SendTo(event.peer));
            }

            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT:
        {
            std::cout << "disconnect event ip " << ToReadableString(event.peer->address) << "\n";
            if (m_serverAddress == event.peer->address)            {
                
                std::cout << "We Disconnected from server: " << ToReadableString(event.peer->address) << "\n";
            }
            if (m_state == ServerConnectionState::Connecting)
            {
                std::cout << "Timeout connecting to server: " << "\n";
            }
            callbacks.Disconnected();
            m_state = ServerConnectionState::Idle;
            m_server = nullptr;
            break;
        }
        }
    }
}