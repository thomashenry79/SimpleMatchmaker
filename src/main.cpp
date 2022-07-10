#include <stdio.h>
#include <enet/enet.h>
#include <stdint.h>
#include <chrono>
#include <string>
#include <map>
#include <vector>
#include "StateMachine.h"
#include "Message.h"
#include "Connections.h"
#include "Sender.h"
#include <iostream>



int main(int argc, char** argv)
{
     Connections connections;

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
    local = enet_host_create(&address, ENET_PROTOCOL_MAXIMUM_PEER_ID, 0, 0, 0);
    if (local == NULL) {
        printf("An error occurred while trying to create an ENet local.\n");
        exit(EXIT_FAILURE);
    }


    // loop
    bool loop = true;
   

    while (loop) {

        connections.Update();
        while (enet_host_service(local, &event, 1) > 0)
        {
            char fromIP[40];
            enet_address_get_host_ip(&event.peer->address, fromIP, 40);

            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
            {                
                printf("We accepted a connection from %s:%u\n",
                    fromIP,
                    event.peer->address.port);
                connections.NewConnection(event.peer);
                    
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE:
            {
                printf("Message from %s:%u, msg is:",
                    fromIP,
                    event.peer->address.port);
                std::string msg(event.packet->data, event.packet->data + event.packet->dataLength);
                std::cout << msg << "\n";
                connections.ReceiveMessage(event.peer, event.packet->data, event.packet->dataLength);                
                enet_packet_destroy(event.packet);

                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT:
            {
                printf("Disconnected from %s:%u\n",
                    fromIP,
                    event.peer->address.port);
                connections.LostConnection(event.peer);
                break;
            }
            }
        }

        

    }

    enet_host_destroy(local);
    enet_deinitialize();
}