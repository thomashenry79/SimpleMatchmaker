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
   // printf("Internal IP: %s\n", IP);
    return addr.S_un.S_addr;
}
using LocalHostPtr = std::unique_ptr<ENetHost, void(*)(ENetHost*)>;
bool operator==(const ENetAddress& lhs, const ENetAddress& rhs)
{
    return lhs.host == rhs.host && lhs.port == rhs.port;
}
class EnetPacketKiller
{
public:
    EnetPacketKiller(ENetPacket* packet) : m_packet(packet) {}
    ~EnetPacketKiller()  {enet_packet_destroy(m_packet);}
    ENetPacket* m_packet;
};
class EnetInitialiser
{
public:
    EnetInitialiser() : code(enet_initialize()){}
    ~EnetInitialiser()
    {
        if(code==0)
            enet_deinitialize();
    }
    int code;
};
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
    EnetInitialiser enetInitGuard;


  
    auto localIP = LocalIP();
 
    // -- vars
    
   
    ENetEvent event;
   
    // -- loc
    ENetAddress serverAddress{ 0,0 };
    ENetAddress localAddress{ 0,0 };

 
  //  local = enet_host_create(&localAddress, 2, 0, 0, 0);
    LocalHostPtr local ( enet_host_create(&localAddress, 3, 0, 0, 0), enet_host_destroy) ;

    if (!local) {
        printf("An error occurred while trying to create an ENet local.\n");
        exit(EXIT_FAILURE);
    }
    enet_socket_get_address(local->socket, &localAddress);
    localAddress.host = localIP;
    std::cout  << "Local IP address is " <<  ToReadableString(localAddress) << "\n";


    enet_address_set_host_ip(&serverAddress, serverIP.c_str());
    serverAddress.port = 19604;
    ENetPeer* server = enet_host_connect(local.get(), &serverAddress, 0, 0);    
    
  
    std::vector<ENetAddress> peerAddresses;
    std::vector<ENetPeer*> peerCandidates;
    std::vector<ENetPeer*> peerConnections;

    // loop
    bool loop = true;
    int pongs = 0;
    int loopCount = 0;
    int nPunches = 0;
    bool connectedToServer = false;
    bool gotPeerDetails = false;
    bool holePunching = false;
    bool attemptConnection = false;

    std::string peerDetails;
    while (loop) {

        loopCount++;
        if (attemptConnection && !connectedToServer)
        {
            // initiate conncetion to peer
            printf("Try to connect to peers: \n");
            for (auto& ad : peerAddresses)
            {
                peerCandidates.push_back(enet_host_connect(local.get(), &ad, 0, 0));
                std::cout << ToReadableString(ad) << "\n";
            }
            std::cout << "\n";
            holePunching = true;
            attemptConnection = false;
        }

        if (holePunching && (loopCount % 100 == 0))
        {
            for (auto& peer : peerCandidates)
                SendMessage(peer, std::string("INFO:Punch"));

          //  std::cout << "Send a punch\n";
        }

            // send packet
            if (_kbhit()) {
                if (_getch() == '1')
                {

                    if (peerConnections.size())
                    {
                      std::cout << "Send INFO:Ping\n";
                     SendMessage(peerConnections[0], "INFO:Ping");
                    }

                }
                else if (_getch() == 'q')
                {
                    loop = false;
                }
        }
        while (enet_host_service(local.get(), &event, 1) > 0)
        {
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
            {
                std::cout << "connect event, ip:  " << ToReadableString(event.peer->address) << "\n";

                if (serverAddress == event.peer->address) {
                    printf("We connected to the server\n");
                    connectedToServer = true;
                    SendMessage(server, std::string("VERSION:0.01"));
                    SendMessage(server, std::string("INFO:")+ToString(localAddress));
                    SendMessage(server, std::string("LOGIN:")+name);
                }
                else if(contains(peerAddresses,event.peer->address) )
                {
                    std::cout << "Connected to a Peer \n";
                 
                    peerConnections.push_back(event.peer);
                    if (peerConnections.size() == 1)
                    {
                        holePunching = false;
                        printf("Hole punched, use this connection\n");
                        if (contains(peerCandidates, event.peer)) {
                            eraseAndRemove(peerCandidates, event.peer);
                            std::cout << "Peer was in our candidate list: we connected to them\n";
                        }
                        else
                            std::cout << "Peer was NOT in our candidate list: they connected to us\n";

                        Message::Make(MessageType::Info, "hello mate my name is " + name).OnData(Sender(event.peer));
                    }
                    else
                    {
                        printf("Redundant connection established\n");
                    }
                }
                else
                {
                    std::cout << "Connected to unknown peer...... bin it\n";
                    enet_peer_disconnect(event.peer,0);
                }
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE:
            {
                EnetPacketKiller guard(event.packet);

                auto msg = Message::Parse(event.packet->data, event.packet->dataLength);                  
                printf("We received a message: ");
                msg.ToConsole();
                if (msg.Type() == MessageType::Start)
                {
                    gotPeerDetails = TryParseIPAddressList(msg.Content(),peerAddresses);
                    
                    std::cout << "IPs of peers in message: ";
                    for(const auto& peerAddress : peerAddresses)
                        std::cout<< ToReadableString(peerAddress) << "\n"; 
                    attemptConnection = true;
                }
                if (msg.Type() == MessageType::Info)
                {
                    if(!strcmp(msg.Content(),"Ping"))
                        SendMessage(peerConnections[0], "INFO:Pong");
                }

                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT:
            {
                std::cout << "disconnect event ip " << ToReadableString(event.peer->address) << "\n";
                if (serverAddress == event.peer->address)
                {
                    std::cout << "We Disconnected from server: " << ToReadableString(event.peer->address) << "\n";
                    connectedToServer = false;
                }
               else if (contains(peerConnections,event.peer))
                {
                    if(event.peer== peerConnections[0])
                        std::cout << "We Disconnected from peer: " << ToReadableString(event.peer->address) << "\n";
                    else
                        std::cout << "We Disconnected from redundant connection: " << ToReadableString(event.peer->address) << "\n";
                    eraseAndRemove(peerConnections, event.peer);
                }
                else if (contains(peerCandidates, event.peer))
                {
                      std::cout << "Abort connection attempt to " << ToReadableString(event.peer->address) << "\n";
                      eraseAndRemove(peerCandidates, event.peer);
                }
                else
                {
                      std::cout << "Disconnect from unknown\n";
                }
                break;
            }
            }
        }



    }
    enet_deinitialize();
}