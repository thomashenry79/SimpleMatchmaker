#include <stdio.h>
#include <enet/enet.h>
#include <stdint.h>
#include <chrono>
#include <string>
#include <map>
#include <vector>
#include <iostream>



int main(int argc, char** argv)
{
    //// general setting
    if (argc != 3) {
         printf("invalid command line parameters\n");
         printf("usage: app localPort name\n");
         return 0;
     }
    // set ip address and port    

     std::string local_port(argv[1]);
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

  

    local = enet_host_create(NULL, 1, 0, 0, 0);
    if (local == NULL) {
        printf("An error occurred while trying to create an ENet local.\n");
        exit(EXIT_FAILURE);
    }

    enet_address_set_host_ip(&address, "127.0.0.1");
    address.port = 19604;// atoi(local_port.c_str());
    ENetPeer* server = enet_host_connect(local, &address, 0, 0);

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
               
                printf("We connected to %s:%u\n",
                    fromIP,
                    event.peer->address.port);
                std::string message = "LOGIN:thd79";
                ENetPacket* packetPong = enet_packet_create(message.c_str(), message.length(), ENET_PACKET_FLAG_RELIABLE);
                enet_peer_send(server, 0, packetPong);
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE:
            {
                std::string message(event.packet->data, event.packet->data + event.packet->dataLength);
                printf("We received a message: %s\n", message.c_str());
              
                enet_packet_destroy(event.packet);

                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT:
            {
                printf("We disconncted\n");
                break;
            }
            }
        }



    }

    enet_host_destroy(local);
    enet_deinitialize();
}