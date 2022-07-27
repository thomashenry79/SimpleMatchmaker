#pragma once
#include <enet/enet.h>
#include <string>
#include "States.h"
#include "StateMachine.h"
#include <iostream>
#include "IConnections.h"
#include "UserChangedStateVisitor.h"
class Message;


class User
{
public:

    User(ENetPeer* peer, IConnections* connections);
    ~User();

    const ENetAddress& LocalIP() const { return m_localIP; }
    const std::string& Name() const { return m_name; }
    const std::string& Version() const { return m_version; }
    ENetPeer* Peer() { return m_peer; }



    void DisconnectUser(const std::string& reason);
    void SendInfoMessage(const std::string& msg) const;

    void OnMessage(const Message& msg);


    bool TrySetNameAndLogIn(const std::string& name);
    bool TrySetVersion(const std::string& name);
    bool TrySetLocaIPAddress(const std::string& data);
    bool CreateGame(const std::string& data);

    bool RequestToJoin(const std::string& data);
    bool LeaveGame(const std::string& data);
   
    void Eject(const std::string& data);
    void Approve(const std::string& data);
    template <class Visitor>
    void Visit(Visitor& v) const
    {
        m_fsm.VisitState(v);
    }
    template<typename State, typename... Args>
    void ChangeState(Args&&... args)
    {
        std::cout << "User " << m_peer << " entered state " << typeid(State).name() <<"\n";
        m_fsm.ChangeState<State>(std::forward<Args>(args)...);
        m_connections->OnUserChangeState(this);
    }
    
private:
    ENetAddress m_localIP;
    IConnections* m_connections;
    ENetPeer* m_peer;
    std::string m_name;
    std::string m_version;
    StateMachine< 
        KickedOffState,
        WatingForVersionState, 
        WatingForLoginState, 
        WatingForLocalIPState,
        LoggedInState, 
        OpenedGameState, 
        JoinedOpenGame,
        PendingJoinState> m_fsm;

private:
    // No copying or moving.
    User(const User&) = delete;
    User& operator=(const User&) = delete;
    User(const User&& other) = delete;
    User& operator=(const User&& other) = delete;
};