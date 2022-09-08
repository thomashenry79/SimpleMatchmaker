#include "User.h"
#include "Message.h"
#include "Connections.h"
#include "Sender.h"
#include "Utils.h"

User::User(ENetPeer* peer, IConnections* connections) :
    m_peer(peer),
    m_connections(connections)
{
    ChangeState <WatingForVersionState>(this);    
}

User::~User()
{
    // tell connections
}
void User::DisconnectUser(const std::string& reason)
{
    std::cout << "Kick user off: user " << reason << "\n";
    SendInfoMessage(reason);
    ChangeState<KickedOffState>();
    enet_peer_disconnect_later(m_peer, 0);
}

void User::OnMessage(const Message& msg) {
    m_fsm.ReceiveMessage(msg);
}

void User::SendInfoMessage(const std::string& msg) const{
    Message::Make(MessageType::Info, msg).OnData(SendTo(m_peer));
}

bool User::TrySetNameAndLogIn(const std::string& name)
{
    if (!m_connections->VerifyName(name))
    {
        DisconnectUser("Username already in use");
        return false;
    }

    SendInfoMessage("User logged in ok, your name is " + name 
        +  "\nyour external IP is " + ToReadableString(m_peer->address) 
        + "\nyour local IP is " + ToReadableString(m_localIP) + "\n");
    m_name = name;

    std::cout << ToReadableString(m_peer->address) << " is called " << m_name << "\n";
    Message::Make(MessageType::Login, m_name).OnData(SendTo(Peer()));
    m_connections->BroadcastMessage(Message::Make(MessageType::Info, "Player connected"));
 

    return true;
}

bool User::TrySetLocaIPAddress(const std::string& data)
{
    return TryParseIPAddress(data, m_localIP);
}

bool User::TrySetVersion(const std::string& version)
{
    if (version.length() == 0)
    {
        DisconnectUser("Invalid version");
        return false;
    }

    m_version = version;
    return true;
}


bool User::CreateGame(const std::string& data)
{
    try
    {
        auto s = stringSplit(data,',');
        if (s.size() != 2)
            throw std::exception();
        int min_players = std::stoi(s[0]);
        int max_players = std::stoi(s[1]);
        
        if (m_connections->OpenGame(this, min_players, max_players))
        {
            Message::Make(MessageType::Create,Name()).OnData(SendTo(Peer()));
            m_connections->BroadcastMessage(Message::Make(MessageType::Info, "New Game created"));
            m_connections->BroadcastOpenGames();
            return true;
        }
        return false;
    }
    catch (...)
    {
        DisconnectUser("Create game message should be format: <CREATE:M,N> M-min num playes, N-max numplayers");
        return false;
    }
}
void User::Eject(const std::string& data)
{
    m_connections->Eject(this, data);
}

void User::Approve(const std::string& data)
{
    if (m_connections->Approve(this, data))
        Message::Make(MessageType::Approve, data).OnData(SendTo(m_peer));
}
bool User::RequestToJoin(const std::string& data)
{
    auto creatorName = data;
    return m_connections->RequestToJoin(this,creatorName);
}
bool User::LeaveGame(const std::string& data)
{
    data;
    m_connections->RemoveUserFromAnyGames(this);
    return true;
}

bool User::TryStartGame()
{
    return m_connections->StartGame(this);
}