#pragma once
#include "UserMap.h"
#include "IConnections.h"
#include <string>
#include "Utils.h"
#include "Game.h"
class User;
class Message;


class Connections : public IConnections
{
public:
    Connections(uint16_t port);
    void NewConnection(ENetPeer* peer);
    void LostConnection(ENetPeer* peer);
    void ReceiveMessage(ENetPeer* peer, const unsigned char* data, size_t len);
    void Update();
    bool VerifyName(const std::string& name) override ;
    bool VerifyVersion(const std::string& version) override;
    void OnUserChangeState(class User*) override;
    ENetHost* Host() { return m_host.get(); }
    void BroadcastMessage(const class Message& m) const override;
    void BroadcastActiveUsers() const override;
    void BroadcastOpenGames() const override;
    bool OpenGame(User* creator, int min_players, int max_players) override;
    bool RequestToJoin(User* requestor, const std::string& data) override;
    void RemoveUserFromAnyGames(User* user) override;
    void Eject(User* owner, const std::string& other) override;
    bool Approve(User* owner, const std::string& other) override;
    bool StartGame(User* owner) override;
private:
    User* UserByName(const std::string& name);
    std::vector<Game> m_games;
    std::vector<User*> ActivePlayers() const;
    ENetAddress address;
    ENetHostPtr m_host;
    UserMap m_users;

};