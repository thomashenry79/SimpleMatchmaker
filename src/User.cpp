#include "User.h"
#include "Message.h"
#include "Connections.h"
#include "Sender.h"

User::User(ENetPeer* peer, Connections* connections) :
    m_peer(peer),
    m_connections(connections)
{
    ChangeState <WatingForVersionState>(this);
}

void User::DisconnectUser(std::string reason)
{
    std::cout << "Kick user off: user " << reason << "\n";
    SendInfoMessage(reason.c_str());
    ChangeState<KickedOffState>();
    enet_peer_disconnect_later(m_peer, 0);
}

void User::OnMessage(const Message& msg) {
    m_fsm.ReceiveMessage(msg);
}

void User::SendInfoMessage(const char* msg) const{
    Message::Make(MessageType::Info, msg).OnData(Sender(m_peer));
}

bool User::TrySetName(std::string name)
{
    if (!m_connections->VerifyName(name))
    {
        DisconnectUser("Username already in use");
        return false;
    }

    SendInfoMessage("User logged in ok");
    m_name = name;
    return true;
}

bool User::TrySetVersion(std::string version)
{
    if (version.length() == 0)
    {
        DisconnectUser("Invalid version");
        return false;
    }

    m_version = version;
    return true;
}