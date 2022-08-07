#pragma once
#include "States.h"
#include "IConnections.h"

class UserChangedStateVisitor {
public:
    UserChangedStateVisitor(IConnections& connections) : m_connections(connections) {};
    void Visit(const JoinedOpenGame&);
    void Visit(const OpenedGameState&);
    void Visit(const LoggedInState&);
    void Visit(const WatingForLocalIPState&);
    void Visit(const WatingForLoginState&);
    void Visit(const WatingForVersionState&);
    void Visit(const KickedOffState&);
    void Visit(const PendingJoinState&);
private:
    IConnections& m_connections;
};
