#include "Connections.h"
#include "User.h"
#include "Message.h"
#include "Sender.h"
#include "Utils.h"
#include "States.h"
#include <algorithm>
#include <iostream>
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
    std::cout << "removed user " << users.size() << " connected\n";
}

class LoggedInUsers
{
public:
    LoggedInUsers(std::vector<User*>& users) : m_users(users){}
    std::vector<User*>& m_users;
    template <class T> void Visit(const T&){};
    template <> void Visit(const LoggedInState& s){m_users.push_back(s.m_user);};
};
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
template <class Visitor>
void VisitAllUsers(std::map<ENetPeer*, std::unique_ptr<User>>& users,Visitor&v)
{
    for(auto& pair : users)
        pair.second->Visit(v);
}
void Connections::Update()
{
    std::vector<User*> loggedInUsers;
    LoggedInUsers visitor(loggedInUsers);
    VisitAllUsers(users,visitor);

    if ( loggedInUsers.size() == 2)
    {
        auto& peer1 = loggedInUsers[0];
        auto& peer2 = loggedInUsers[1];
        if(peer1->Name().length() && peer2->Name().length()){

            Message::Make(MessageType::Start, ToString(peer1->Peer()->address)).OnData(Sender(peer2->Peer()));
            Message::Make(MessageType::Start, ToString(peer2->Peer()->address)).OnData(Sender(peer1->Peer()));
            peer1->DisconnectUser("Have fun, player1");
            peer2->DisconnectUser("Have fun, player2");
        }
    }
}