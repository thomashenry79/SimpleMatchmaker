#pragma once
#include <enet/enet.h>
#include <string>
#include "States.h"
#include "StateMachine.h"


class Connections;
class Message;


class User
{
public:

    User(ENetPeer* peer, Connections* connections) : 
        m_peer(peer), 
        m_connections(connections)
    {
        ChangeState <AwaitingVerificatonState>(this);    
    };
    ~User();
    User(const User&) = delete;
    User& operator=(const User&) = delete;

    User(const User&& other)=delete;
    User& operator=(const User&& other)= delete;


    void DisconnectUser(std::string reason);
    void OnMessage(const Message& msg);
    void SendInfoMessage(const char* msg) const;
    bool TrySetName(std::string name);
    const std::string& Name() const { return m_name; }

    template<typename State, typename... Args>
    void ChangeState(Args&&... args)
    {
        std::cout << "User " << m_peer << " entered state " << typeid(State).name() <<"\n";
        m_fsm.ChangeState<State>(std::forward<Args>(args)...);
    }
   
private:
    Connections* m_connections;
    ENetPeer* m_peer;
    std::string m_name;
   
    StateMachine< KickedOffState,AwaitingVerificatonState, LoggedInState, OpenedGameState, JoinedOpenGame> m_fsm;
};