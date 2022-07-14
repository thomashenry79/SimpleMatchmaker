#pragma once
#include "States.h"
#include <enet/enet.h>
class Connections;

class UserChangedStateVisitor {
public:
    UserChangedStateVisitor(Connections& connections, ENetHost* host) : m_connections(connections),m_host(host) {};
    void Visit(const JoinedOpenGame&);
    void Visit(const OpenedGameState&);
    void Visit(const LoggedInState&);
    void Visit(const WatingForLocalIPState&);
    void Visit(const WatingForLoginState&);
    void Visit(const WatingForVersionState&);
    void Visit(const KickedOffState&);
private:
    Connections& m_connections;
    ENetHost* m_host;
};
