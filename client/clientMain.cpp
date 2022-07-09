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
void SendMessage(ENetPeer* to, const std::string& message)
{
    enet_peer_send(
        to, 
        0, 
        enet_packet_create(message.c_str(), message.length(), ENET_PACKET_FLAG_RELIABLE)
    );
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
    ENetAddress address;
    ENetHost* local;
    ENetEvent event;
   
    // -- loc

  
    /*enet_address_set_host_ip(&address, "127.0.0.1");*/
    address.host = 0;
    address.port = 0;
    local = enet_host_create(&address, 1, 0, 0, 0);
    if (local == NULL) {
        printf("An error occurred while trying to create an ENet local.\n");
        exit(EXIT_FAILURE);
    }

    enet_address_set_host_ip(&address, serverIP.c_str());
    address.port = 19604;// atoi(local_port.c_str());
    ENetPeer* server = enet_host_connect(local, &address, 0, 0);
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
        if (gotPeerDetails && !connected)
        {
            // initiate conncetion to peer
            peer = enet_host_connect(local, &address, 0, 0);
            printf("Try to connect to peer\n");
        }
        while (enet_host_service(local, &event, 1) > 0)
        {
            char fromIP[40];
            enet_address_get_host_ip(&event.peer->address, fromIP, 40);

            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
            {
                if (server == event.peer) {
                    printf("We connected to server %s:%u\n",
                        fromIP,
                        event.peer->address.port);
                    SendMessage(server, std::string("VERSION:0.01"));
                    SendMessage(server, std::string("LOGIN:") + name);
                }
                else
                {
                    printf("We connected to pper %s:%u\n",
                        fromIP,
                        event.peer->address.port);

                    Message::Make(MessageType::Info, "hello Dipshit").OnData(Sender(event.peer));
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
                    gotPeerDetails = msg.TryParseIPAddress(address.host, address.port);
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