#include "Connections.h"
#include "User.h"
#include "Message.h"
#include "Sender.h"
#include "Utils.h"
#include "States.h"
#include <algorithm>
#include <iostream>
#include "Sender.h"
#include "UserChangedStateVisitor.h"
Connections::Connections(uint16_t port) : 
    address{ ENET_HOST_ANY,port },
    m_host(enet_host_create(&address, ENET_PROTOCOL_MAXIMUM_PEER_ID, 0, 0, 0), enet_host_destroy)
{  
    if (!m_host) {
        throw "An error occurred while trying to create an ENet local.\n";
    }
}

void Connections::OnUserChangeState(class User* user)
{
    UserChangedStateVisitor v(*this);
    user->Visit(v);
}

void Connections::NewConnection(ENetPeer* peer)
{
    if (!users.count(peer))
        users.insert({ peer, std::make_unique<User>(peer,this) });

    std::cout << "added user " << users.size() << " connected\n";
}

void Connections::LostConnection(ENetPeer* peer)
{
    if (users.count(peer))
        users.erase(peer); 
    BroadcastMessage(Message::Make(MessageType::Info, "Player disconnected"));
    BroadcastActiveUsers();
    std::cout << "removed user " << users.size() << " connected\n";
}



void Connections::ReceiveMessage(ENetPeer* peer, const unsigned char* data, size_t len)
{
    if (!users.count(peer))
        return;
    auto& user = users.at(peer);
    try
    {
        user->OnMessage(Message::Parse(data, len));
    }
    catch (BadMessageException&)
    {
        // Tell the client their message was bogus, then kick them off. Should never happen if the client works properly
        user->DisconnectUser("Badly formed message.");
    }

}



bool Connections::VerifyName(const std::string& name)
{
    auto success = std::count_if(users.begin(), users.end(), [&](auto& p) {
        return p.second->Name() == name; }) == 0;
     
    return success;
}

bool Connections::VerifyVersion(const std::string& version)
{
    // TODO check vesion
    return true;
}

class IsUserActive
{
    bool m_bActive;
public:
    bool operator()() const {return m_bActive; }
    template <class T> void Visit(const T& s) { m_bActive = true; }
    template <> void Visit(const WatingForLocalIPState& s) { m_bActive = false; };
    template <> void Visit(const WatingForLoginState& s) { m_bActive = false; }
    template <> void Visit(const WatingForVersionState& s) { m_bActive = false; }
    template <> void Visit(const KickedOffState& s) { m_bActive = false; }
};

class ActiveUsersListMaker
{
public:
    std::vector<User*> m_ActiveUsers;
    template <class T> void Visit(const T& s) {
        IsUserActive isActive;
        isActive.Visit(s);
        if(isActive())
            m_ActiveUsers.push_back(s.m_user);
    };
};

std::vector<User*> Connections::ActivePlayers() const
{
    ActiveUsersListMaker v;
    return VisitMap(users, v).m_ActiveUsers;
}

std::vector<ENetPeer*> AsPeers(const std::vector<User*>& users)
{
    std::vector<ENetPeer*> peers(users.size());
    std::transform(users.begin(), users.end(), peers.begin(), [](User* u) {return u->Peer(); });
    return peers;
}

void Connections::BroadcastMessage(const Message& m) const
{
    auto activeUsers = AsPeers(ActivePlayers());
    m.OnData(SendTo(activeUsers));
}
void Connections::BroadcastActiveUsers() const
{
    auto activeUsers = ActivePlayers();

    std::string userList;
    for (const auto& u : activeUsers)
        userList += u->Name() + ",";

    BroadcastMessage(Message::Make(MessageType::PlayersActive, userList));
}
void Connections::Update()
{
   
}