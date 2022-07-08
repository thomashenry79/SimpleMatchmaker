#include "User.h"
#include "Message.h"
#include "Connections.h"
#include "Sender.h"
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
void User::SendInfoMessage(const char* msg) const
{
    Message::Make(MessageType::Info, msg).OnData(Sender(m_peer));
}
bool User::TrySetName(std::string name)
{
    if (!m_connections->VerifyName(name))
        return false;
    SendInfoMessage("User logged in ok");
    m_name = name;
    return true;
}
User::~User()
{
}
//
//User::User(const User&& other)
//{
//    *this = std::move(other);
//}
//User& User::operator=(const User&& other)
//{
//    m_peer = other.m_peer;
//    m_connections = other.m_connections;
//    m_name = other.m_name;
//    m_fsm = other.m_fsm;
//    m_fsm.SetOwner(this);
//    return *this;
//}