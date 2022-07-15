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

bool User::TrySetName(const std::string& name)
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