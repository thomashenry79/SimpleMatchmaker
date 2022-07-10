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
#include <ws2tcpip.h>
#include <conio.h>
uint32_t LocalIP() {
  char host[256];
    char* IP;
    struct hostent* host_entry;
    int hostname;
    hostname = gethostname(host, sizeof(host)); //find the host name
 
    host_entry = gethostbyname(host); //find host information

    auto addr = *((struct in_addr*)host_entry->h_addr_list[0]);
    IP = inet_ntoa(addr); //Convert into IP string
    printf("Current Host Name: %s\n", host);
    printf("Internal IP: %s\n", IP);
    return addr.S_un.S_addr;
}

bool operator==(const ENetAddress& lhs, const ENetAddress& rhs)
{
    return lhs.host == rhs.host && lhs.port == rhs.port;
}
int main(int argc, char** argv)
{
    //// general setting
    if (argc != 2) {
         printf("invalid command line parameters\n");
         printf("usage: Client <name>\n");
         return 0;
     }
    // set ip address and port    
    std::string serverIP("82.6.1.150");
   // std::string serverIP("192.168.0.50");
     std::string name(argv[1]);

    // init
    // -- enet
    if (enet_initialize() != 0) {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }


  
    auto localIP = LocalIP();
 
    // -- vars
    
    ENetHost* local;
    ENetEvent event;
   
    // -- loc

    ENetAddress serverAddress;
    ENetAddress peerAddress;
    ENetAddress localAddress;


    localAddress.host = 0;
    localAddress.port = 0;
    local = enet_host_create(&localAddress, 2, 0, 0, 0);
    if (local == NULL) {
        printf("An error occurred while trying to create an ENet local.\n");
        exit(EXIT_FAILURE);
    }
    enet_socket_get_address(local->socket, &localAddress);
    printf("Local Port: %d\n", localAddress.port);
    enet_address_set_host_ip(&serverAddress, serverIP.c_str());
    serverAddress.port = 19604;

    ENetPeer* server = enet_host_connect(local, &serverAddress, 0, 0);
    
    
    ENetPeer* peer = nullptr;
    ENetPeer* actualPeer = nullptr;
    // loop
    bool loop = true;
    int pongs = 0;
    int loopCount = 0;
    int nPunches = 0;
    bool connected = false;
    bool gotPeerDetails = false;
    bool holePunching = false;
    std::string peerDetails;
    while (loop) {

        loopCount++;
        if (gotPeerDetails && !connected && !peer)
        {
            // initiate conncetion to peer
            peer = enet_host_connect(local, &peerAddress, 0, 0);
            printf("Try to connect to peer\n");
            holePunching = true;
        }

        if (holePunching && (loopCount % 100 == 0))
        {
            SendMessage(peer, std::string("INFO:Punch"));
         //   std::cout << "Send a punch\n";
        }
        Sleep(1);
        if (actualPeer)
        {
            // send packet
            if (_kbhit()) {
                if (_getch() == '1')
                {
                    std::cout << "Send INFO:Ping\n";
                    SendMessage(actualPeer, "INFO:Ping");
                }
            }
        }
        while (enet_host_service(local, &event, 0) > 0)
        {
            

            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
            {
               

                if (serverAddress == event.peer->address) {
                    printf("We connected to the server\n");
                    SendMessage(server, std::string("VERSION:0.01"));
                    SendMessage(server, std::string("LOGIN:") + name);
                }
                else if(peerAddress ==event.peer->address && !actualPeer)
                {
                    actualPeer = event.peer;
                    holePunching = false;
                    printf("We connected to a peer, hole punched\n");

                    Message::Make(MessageType::Info, "hello mate my name is " + name).OnData(Sender(event.peer));
                }
                else
                {
                   // std::cout << "Connected to uneeded connection \n";
                  //  enet_peer_disconnect(event.peer,0);
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
                if (msg.Type() == MessageType::Info)
                {
                    if(!strcmp(msg.Content(),"Ping"))
                        SendMessage(actualPeer, "INFO:Pong");
                }
                enet_packet_destroy(event.packet);

                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT:
            {
                connected = false;
                if(serverAddress == event.peer->address)
                    std::cout << "We Disconnected from server: " << ToReadableString(event.peer->address) << "\n";
                else if (peerAddress == event.peer->address)
                {
                    if (actualPeer == event.peer) {
                        actualPeer = 0;
                        std::cout << "We Disconnected from peer: " << ToReadableString(event.peer->address) << "\n";
                    }
                    else
                    {
                     ///   std::cout << "Drop unused connection\n";
                    }
                }
                break;
            }
            }
        }



    }

    enet_host_destroy(local);
    enet_deinitialize();
}