#include <stdio.h>
#include <enet/enet.h>
#include <stdint.h>
#include <chrono>
#include <string>
#include <map>
#include <vector>
#include "StateMachine.h"
using namespace std::chrono;

enum class UserState
{
    AwaitingVerificatonState, LoggedInState, OpenedGameState, JoinedOpenGame
};

struct AwaitingVerificatonState
{
    void ReceiveMessage(const std::string& msg)
    {
        msg;
    }
};

struct LoggedInState
{
    void ReceiveMessage(const std::string& msg)
    {
        msg;
    }
};
struct OpenedGameState
{
    void ReceiveMessage(const std::string& msg)
    {
        msg;
    }
};

struct JoinedOpenGame
{
    void ReceiveMessage(const std::string& msg)
    {
        msg;
    }
};

enum MessageType
{
    Login,
    Max
};

const char* MessagePrefixes[MessageType::Max] = { "LOGIN:" };



class User
{
    
public:
    User(ENetPeer* peer) : m_peer(peer) {};
    void OnMessage(const std::string& msg) { 
        m_fsm.ReceiveMessage(msg);
    }
    const std::string& Name() const { return m_name; }
private:
    ENetPeer* m_peer;
    std::string m_name;
    UserState m_state = UserState::AwaitingVerificatonState;
    StateMachine< AwaitingVerificatonState, LoggedInState, OpenedGameState, JoinedOpenGame> m_fsm;
};


//class OpenGame
//{
//    User owner;
//    std::vector<User> others;
//};

class Connections
{
public:
    void NewConnection(ENetPeer* peer) 
    {
        if (users.count(peer))
            throw "Peer already in connected list";
        else
        {
            users.insert({ peer, User(peer) });
        }
    }

    void LostConnection(ENetPeer* peer)
    {
        if (users.count(peer))
            users.erase(peer);
        else
        {
            throw "unknown peer";
        }
    }

    void ReceiveMessage(ENetPeer* peer, const std::string& message)
    {
        if (users.count(peer))
            users.at(peer).OnMessage(message);
        else
        {
            throw "unknown peer";
        }
    }

    std::map<ENetPeer*, User> users;
};
int main(int argc, char** argv)
{
    // general setting
     if (argc != 4) {
         printf("invalid command line parameters\n");
         printf("usage: SimpleMatchmakerServer localPort\n");
         return 0;
     }
     Connections connections;
    // set ip address and port    

     std::string local_port(argv[1]);

    // init
    // -- enet
    if (enet_initialize() != 0) {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }

    // -- vars
    ENetAddress address;
    ENetHost* local;
    ENetEvent event;

    // -- loc
    
    address.host = ENET_HOST_ANY;
    address.port = atoi(local_port.c_str());
    local = enet_host_create(&address, ENET_PROTOCOL_MAXIMUM_PEER_ID, 0, 0, 0);
    if (local == NULL) {
        printf("An error occurred while trying to create an ENet local.\n");
        exit(EXIT_FAILURE);
    }


    // loop
    bool loop = true;
    int pongs = 0;
    int loopCount = 0;
    int nPunches = 0;

    while (loop) {

        loopCount++;
        while (enet_host_service(local, &event, 1) > 0)
        {
            char fromIP[40];
            enet_address_get_host_ip(&event.peer->address, fromIP, 40);

            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
            {
                connections.NewConnection(event.peer);
                printf("We accepted a connection from %s:%u\n",
                    fromIP,
                    event.peer->address.port);
                    
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE:
            {
                std::string message(event.packet->data, event.packet->data + event.packet->dataLength);
                connections.ReceiveMessage(event.peer, message);
                //auto pc = *((PeerCommand*)(event.packet->data));
                //if (pc == PeerCommand::pc_Ping) {

                //    printf("Received ping from %s:%d\n", ip, (int)event.peer->address.port);
                //    auto command = PeerCommand::pc_Pong;
                //   ENetPacket* packetPong = enet_packet_create(&command, sizeof(PeerCommand), ENET_PACKET_FLAG_RELIABLE);
               //    enet_peer_send(event.peer, 0, packetPong);

                //}
                //else if (pc == PeerCommand::pc_Pong)
                //{
                //    auto now = std::chrono::high_resolution_clock::now();
                //    auto msOfRoundTrip = (int)duration_cast<milliseconds>(now - pingSendTime).count();
                //    printf("Received pong number %d from %s:%d, round trip time %dms\n", ++pongs, ip, (int)event.peer->address.port, msOfRoundTrip);
                //}
                //else if (pc == PeerCommand::pc_Punch) // Unexpected, punch messages should happen before the connection, in order to help establish it
                //{
                //    printf("Received punch from %s:%d\n", ip, (int)event.peer->address.port);

                //}
                enet_packet_destroy(event.packet);

                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT:
            {
                connections.LostConnection(event.peer);
                break;
            }
            }
        }

        

    }

    enet_host_destroy(local);
    enet_deinitialize();
}