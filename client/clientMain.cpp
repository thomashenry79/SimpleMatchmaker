#include <stdio.h>
#include <enet/enet.h>
#include <stdint.h>
#include <chrono>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include "Message.h"
#include "Sender.h"
#include "Utils.h"
void SendMessage(ENetPeer* to, const std::string& message)
{
    enet_peer_send(
        to, 
        0, 
        enet_packet_create(message.c_str(), message.length(), ENET_PACKET_FLAG_RELIABLE)
    );
}


bool operator==(const ENetAddress& lhs, const ENetAddress& rhs)
{
    return lhs.host == rhs.host && lhs.port == rhs.port;
}
int main(int argc, char** argv)
{
    //// general setting
    if (argc != 3) {
         printf("invalid command line parameters\n");
         printf("usage: app serverIP localPort name\n");
         return 0;
     }
    // set ip address and port    
    std::string serverIP(argv[1]);
    std::string name(argv[2]);

    // init
    // -- enet
    if (enet_initialize() != 0) {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }

    // -- vars
    
    ENetHost* local;
    ENetEvent event;
   
    // -- loc

    ENetAddress serverAddress;
    ENetAddress peerAddress;
    ENetAddress localAddress;
    /*enet_address_set_host_ip(&address, "127.0.0.1");*/
    localAddress.host = 0;
    localAddress.port = 0;
    local = enet_host_create(&localAddress, 1, 0, 0, 0);
    if (local == NULL) {
        printf("An error occurred while trying to create an ENet local.\n");
        exit(EXIT_FAILURE);
    }

    enet_address_set_host_ip(&serverAddress, serverIP.c_str());
    serverAddress.port = 19604;// atoi(local_port.c_str());
    ENetPeer* server = enet_host_connect(local, &serverAddress, 0, 0);
    ENetPeer* peer = nullptr;
    // loop
    bool loop = true;
    int pongs = 0;
    int loopCount = 0;
    int nPunches = 0;
    bool connected = false;
    bool gotPeerDetails = false;
    std::string peerDetails;
    while (loop) {

        loopCount++;
        if (gotPeerDetails && !connected && !peer)
        {
            // initiate conncetion to peer
            peer = enet_host_connect(local, &peerAddress, 0, 0);
            printf("Try to connect to peer\n");
        }
        while (enet_host_service(local, &event, 1) > 0)
        {
            

            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
            {
               std::cout << "We connected to someone: " << ToReadableString(event.peer->address) <<"\n";

                if (serverAddress == event.peer->address) {
                    printf("We connected to the server\n");
                    SendMessage(server, std::string("VERSION:0.01"));
                    SendMessage(server, std::string("LOGIN:") + name);
                }
                else
                {
                    printf("We connected to a peer\n");

                    Message::Make(MessageType::Info, "hello Dipshit my name is " + name).OnData(Sender(event.peer));
                }
                connected = true;
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE:
            {
                auto msg = Message::Parse(event.packet->data, event.packet->dataLength);                  
                printf("We received a message: ");
                msg.ToConsole();
                if (msg.Type() == MessageType::Start)
                {
                    gotPeerDetails = msg.TryParseIPAddress(peerAddress.host, peerAddress.port);
                    std::cout << "IP of peer in message: " << ToReadableString(peerAddress) << "\n";
                }
                enet_packet_destroy(event.packet);

                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT:
            {
                connected = false;
                printf("We disconncted\n");
                break;
            }
            }
        }



    }

    enet_host_destroy(local);
    enet_deinitialize();
}