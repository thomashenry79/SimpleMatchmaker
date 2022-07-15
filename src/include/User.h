#pragma once
#include <enet/enet.h>
#include <string>
#include "States.h"
#include "StateMachine.h"
#include <iostream>
#include "IConnections.h"
class Message;


class User
{
public:

    User(ENetPeer* peer, IConnections* connections);


    void DisconnectUser(const std::string& reason);
    void OnMessage(const Message& msg);
    void SendInfoMessage(const std::string& msg) const;
    bool TrySetName(const std::string& name);
    bool TrySetVersion(const std::string& name);
    bool TrySetLocaIPAddress(const std::string& data);
    ENetPeer* Peer() { return m_peer; }
    const ENetAddress& LocalIP() const { return m_localIP; }
    const std::string& Name() const { return m_name; }
    const std::string& Version() const { return m_version; }

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
        m_fsm.VisitState(m_connections->UserChangeStateHandler());
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
        JoinedOpenGame> m_fsm;

private:
    // No copying or moving.
    User(const User&) = delete;
    User& operator=(const User&) = delete;
    User(const User&& other) = delete;
    User& operator=(const User&& other) = delete;
};