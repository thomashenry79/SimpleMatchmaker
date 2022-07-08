#include "Connections.h"
#include "User.h"
#include "Message.h"
#include "Sender.h"
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
    return std::count_if(users.begin(), users.end(), [&](auto& p) {
        return p.second->Name() == name; }) == 0;
}

bool Connections::VerifyVersion(const std::string& version)
{
    // TODO check vesion
    return true;
}

void Connections::Update()
{

}