#include <stdio.h>
#include <enet/enet.h>
#include <stdint.h>
#include <chrono>
#include <string>
#include <unistd.h>
using namespace std::chrono;

enum class PeerCommand : uint8_t
{
    pc_None,
    pc_Punch,
    pc_Ping,
    pc_Pong,
};

int main(int argc, char** argv)
{
    // general setting
    if (argc != 4) {
        printf("invalid command line parameters\n");
        printf("usage: udt-test localPort remoteIP remotePort\n");
        return 0;
    }

    // set ip address and port
    
    char local_port[32];
    char remote_ip[32];
    char remote_port[32];

    std::string local_port(argv[1]);
    std::string remote_ip(argv[2]);
    std::string remote_port(argv[3]);

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
    // Only allow incoming connections from the remote IP
    enet_address_set_host_ip(&address, remote_ip);
    address.port = atoi(local_port);
    local = enet_host_create(&address, 2, 1, 0, 0);

    if (local == NULL) {
        printf("An error occurred while trying to create an ENet local.\n");
        exit(EXIT_FAILURE);
    }
    printf("created Host on port %d\n", atoi(local_port));

    auto pingSendTime = std::chrono::high_resolution_clock::now();

    ENetPeer* otherPeer = nullptr;
    ENetPeer* connectedPeer = nullptr;

    address.port = atoi(remote_port);
    otherPeer = enet_host_connect(local, &address, 2, 0);

    if (NULL == otherPeer) {
        printf("An error occurred while trying to create an ENet peer.\n");
        exit(EXIT_FAILURE);
    }
    printf("try to connect to peer host on port %d\n", atoi(remote_port));

    // loop
    bool loop = true;
    int pongs = 0;
    int loopCount = 0;
    int nPunches = 0;

    while (loop) {
        sleep(1);
        loopCount++;
        while (enet_host_service(local, &event, 0) > 0)
        {
            char ip[40];
            enet_address_get_host_ip(&event.peer->address, ip, 40);

            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
            {
                if (!connectedPeer)
                {
                    if (otherPeer == event.peer)
                    {
                        printf("We connected to %s:%u\n",
                            ip,
                            event.peer->address.port);
                    }
                    else
                    {
                        printf("We accepted a connection from %s:%u\n",
                            ip,
                            event.peer->address.port);
                    }
                    connectedPeer = event.peer;
                }

                break;
            }
            case ENET_EVENT_TYPE_RECEIVE:
            {
                auto pc = *((PeerCommand*)(event.packet->data));
                if (pc == PeerCommand::pc_Ping) {

                    printf("Received ping from %s:%d\n", ip, (int)event.peer->address.port);
                    auto command = PeerCommand::pc_Pong;
                    ENetPacket* packetPong = enet_packet_create(&command, sizeof(PeerCommand), ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(event.peer, 0, packetPong);

                }
                else if (pc == PeerCommand::pc_Pong)
                {
                    auto now = std::chrono::high_resolution_clock::now();
                    auto msOfRoundTrip = (int)duration_cast<milliseconds>(now - pingSendTime).count();
                    printf("Received pong number %d from %s:%d, round trip time %dms\n", ++pongs, ip, (int)event.peer->address.port, msOfRoundTrip);
                }
                else if (pc == PeerCommand::pc_Punch) // Unexpected, punch messages should happen before the connection, in order to help establish it
                {
                    printf("Received punch from %s:%d\n", ip, (int)event.peer->address.port);

                }
                enet_packet_destroy(event.packet);

                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT:
            {
                if (connectedPeer == event.peer)
                {
                    printf("Established connection lost\n");
                    loop = false;
                    connectedPeer = nullptr;
                }

                event.peer->data = NULL;

                break;
            }
            default:
            {
                printf("Received event type %d\n", event.type);
            }
            }
        }

        // Until we are connected, send a packet approx 10 times a second in order to try and punch a hole through the NAT
        if (!connectedPeer && (loopCount % 100 == 0))
        {
            auto command = PeerCommand::pc_Punch;
            ENetPacket* packetPunch = enet_packet_create(&command, sizeof(PeerCommand), 0);
            enet_peer_send(otherPeer, 0, packetPunch);
            printf("Send punch %d to remote peer\n", ++nPunches);
        }

        if (connectedPeer)
        {
            // // send packet
            // if (_kbhit()) {
            //     if (_getch() == '1')
            //     {
            //         auto command = PeerCommand::pc_Ping;
            //         ENetPacket* packetPing = enet_packet_create(&command, sizeof(PeerCommand), ENET_PACKET_FLAG_RELIABLE);

            //         enet_peer_send(connectedPeer, 0, packetPing);
            //         pingSendTime = std::chrono::high_resolution_clock::now();
            //         printf("Sent ping\n");
            //     }
            //     else if (_getch() == 'q')
            //     {
            //         enet_peer_disconnect(connectedPeer, 0);
            //         printf("q pressed, exit app\n");
            //     }
            // }
        }

    }

    enet_host_destroy(local);
    enet_deinitialize();
}