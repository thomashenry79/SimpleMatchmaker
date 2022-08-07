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
    if (!m_users.count(peer))
        m_users.insert({ peer, std::make_unique<User>(peer,this) });

    std::cout << "added user, currently " << m_users.size() << " connected\n";
}

void Connections::LostConnection(ENetPeer* peer)
{
    if (m_users.count(peer) == 0)
        return;
    auto& u = m_users.at(peer);

    RemoveUserFromAnyGames(u.get());

    m_users.erase(peer);
    
    BroadcastMessage(Message::Make(MessageType::Info, "Player disconnected"));
    BroadcastActiveUsers();
    std::cout << "removed user, currently " << m_users.size() << " connected\n";
}

void Connections::RemoveUserFromAnyGames(User* user)
{
    auto it = std::find_if(RANGE(m_games), [&](const Game& g) {return g.WasCreatedBy(user); });
    if (it != std::end(m_games))
    {
        it->KillGame();
        Message::Make(MessageType::Leave, "").OnData(SendTo(user->Peer()));
        m_games.erase(it);
        std::cout << "Closed game created by " << user->Name() << "\n";
    }
    else
    {
        for (auto& game : m_games)
        {
            if (game.RemoveJoinedOrPending(user))
            {
                Message::Make(MessageType::Leave, "").OnData(SendTo(user->Peer()));
                return;
            }
        }
    }
}

void Connections::ReceiveMessage(ENetPeer* peer, const unsigned char* data, size_t len)
{
    if (!m_users.count(peer))
        return;
    auto& user = m_users.at(peer);
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
    auto success = std::count_if(m_users.begin(), m_users.end(), [&](auto& p) {
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
    void Visit(const JoinedOpenGame&){ m_bActive = true; }
    void Visit(const OpenedGameState&){ m_bActive = true; }
    void Visit(const LoggedInState&){ m_bActive = true; }
    void Visit(const PendingJoinState&) { m_bActive = true; }
    void Visit(const WatingForLocalIPState&){ m_bActive = false; }
    void Visit(const WatingForLoginState&){ m_bActive = false; }
    void Visit(const WatingForVersionState&){ m_bActive = false; }
    void Visit(const KickedOffState&){ m_bActive = false; }

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
    return VisitMap(m_users, v).m_ActiveUsers;
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
    BroadcastOpenGames();
}
void Connections::BroadcastOpenGames() const
{
    std::string gamelist;
    for (const auto& g : m_games)
        gamelist += g.ShortInfo() + ",";

    BroadcastMessage(Message::Make(MessageType::GamesOpen, gamelist));
}
void Connections::Update()
{
   
}
bool Connections::OpenGame(User* creator, int min_players, int max_players)
{
    m_games.push_back(Game(creator, min_players, max_players));
    return true;
}

bool Connections::RequestToJoin(User* requestor, const std::string& nameOfGame) 
{
    auto it  = std::find_if(RANGE(m_games), [&](const Game& g) {return g.WasCreatedBy(nameOfGame); });
    if (it == std::end(m_games))
        return false;
    auto joined = it->RequestUserJoin(requestor);
    if (joined)
    {
        auto msg = Message::Make(MessageType::Join, requestor->Name()); 
        msg.OnData(SendTo(it->CreatedBy()->Peer()));
        msg.OnData(SendTo(requestor->Peer()));
    }
    return joined;
}

User* Connections::UserByName(const std::string& name) 
{
    auto users = ActivePlayers();
    for (auto it : users)
        if (it->Name() == name)
            return it;

    return nullptr;
}

// owner wants to remove the player specified in the message
void Connections::Eject(User* owner, const std::string& other) 
{
    auto p = UserByName(other);
    if (!p)
        return;
    auto it = std::find_if(RANGE(m_games), [&](const Game& g) {return g.WasCreatedBy(owner); });
    if (it == std::end(m_games))
        return;

    if (it->RemoveJoinedOrPending(p)) {
        Message::Make(MessageType::Eject, owner->Name()).OnData(SendTo(p->Peer()));
        p->ChangeState<LoggedInState>(p);
    }
}

bool Connections::Approve(User* owner, const std::string& other) 
{
    auto it = std::find_if(RANGE(m_games), [&](const Game& g) {return g.WasCreatedBy(owner); });
    if (it == std::end(m_games))
        return false;
   return it->Approve(other);
}

bool Connections::StartGame(User* owner)
{
    auto it = std::find_if(RANGE(m_games), [&](const Game& g) {return g.WasCreatedBy(owner); });
    if (it == std::end(m_games))
        return false;
    bool canStart = it->CanStart();
    
    if (canStart)
    {
        User* p2 = it->FirstJoiner();
        std::string gameInfo1 = std::string("1") + "," + p2->Name() + ","+ ToString(p2->Peer()->address) + "," + ToString(p2->LocalIP());
        std::string gameInfo2 = std::string("2") + "," + owner->Name() + "," + ToString(owner->Peer()->address) + "," + ToString(owner->LocalIP());

        Message::Make(MessageType::Start, gameInfo1).OnData(SendTo(owner->Peer()));
        Message::Make(MessageType::Start, gameInfo2).OnData(SendTo(p2->Peer()));
        RemoveUserFromAnyGames(owner);        
        BroadcastOpenGames();
    }
    return canStart;
}