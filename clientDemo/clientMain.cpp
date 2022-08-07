#include <stdio.h>

#include <stdint.h>
#include <chrono>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include "Message.h"
#include <conio.h>
#include "ServerConnection.h"
#include "Sender.h"





class SimpleDemoPeer
{
public:
    SimpleDemoPeer(uint16_t port) : 
        localAddress{ 0,port },
        local(enet_host_create(&localAddress, 3, 0, 0, 0), 
        enet_host_destroy)
    {
    }
    
    ~SimpleDemoPeer()
    {
        for(auto& peer : peerConnections)
            enet_peer_disconnect(peer, 0);

        if(server && connectedToServer)
            enet_peer_disconnect(server, 0);
        enet_host_flush(local.get());
    }
 
    void Update()
    {
        if (holePunching && (updateCount % 50 == 0))
        {
            for (auto& peer : peerCandidates)
                Message::Make(MessageType::Info, "Peer").OnData(SendTo(peer));
        }

    }
    ENetAddress localAddress{ 0,0 };
    ENetHostPtr local;
    std::vector<ENetAddress> peerAddresses;
    std::vector<ENetPeer*> peerCandidates;
    std::vector<ENetPeer*> peerConnections;
    bool connectedToServer = false;
    ENetPeer* server;
    bool holePunching = false;
    int updateCount = 0;
};

int main(int argc, char** argv)
{
    //// general setting
    if (argc < 2) {
         printf("invalid command line parameters\n");
         printf("usage: Client <name> <IP optional>\n");
         return 0;
     }
    // set ip address and port    

   std::string serverIP(argc >=3 ? argv[2] : "82.6.1.150");
   std::string name(argv[1]);

    // init
    // -- enet
   
   
    EnetInitialiser enetInitGuard;

    ServerConnection serverConnection(serverIP, 19601, name, "SimpleTestApp");
    SimpleDemoPeer client(0);
      

    // loop
    bool loop = true;
    int pongs = 0;
    int loopCount = 0;
    int nPunches = 0;
  
    bool gotPeerDetails = false;
   
    bool attemptConnection = false;
    std::vector<std::string> openGames;
    std::vector<std::string> requestedJoiners;
    std::vector<std::string> joined;

    ServerCallbacks cbs;

    cbs.Connected = []() {std::cout << "Connected to server\n"; };
    cbs.Disconnected = []() {std::cout << "Disconnected from server\n"; };
    cbs.Timeout = []() {std::cout << "Timeout trying to connect to server\n"; };
    cbs.JoinRequestFromOtherPlayer = [](const std::string& userName) { std::cout << userName << "Wants to join\n"; };
    cbs.JoinRequestOK = []() {std::cout << "Waiting for host to accept\n"; };
    cbs.GameCreatedOK = []() {std::cout << "We successfully created a game\n"; };
    cbs.StartP2P = [](const GameStartInfo& i) {std::cout << "Ready to Start game, info:\n" << i.ToString(); };
    cbs.LeftGameOK = []() {std::cout << "We left the game\n"; };
    cbs.RemovedFromGame = []() {std::cout << "We were removed from the game\n"; };
    cbs.UserList = [](const std::vector<std::string>& userNames) 
    {
        std::cout << "Active Users: ";
        for (const auto& u : userNames)
            std::cout << u << ", ";
        std::cout << "\n";
    };

    cbs.OpenGames = [&](const std::vector<std::string>& games)
    {
        openGames = games;
        std::cout << "Open Games: ";
        for (const auto& u : openGames)
            std::cout << u << ", ";
        std::cout << "\n";
    };

    cbs.GameInfo = [&](const GameInfoStruct& info)
    {
        requestedJoiners = info.requested;
        joined = info.joined;
        std::cout << "GameInfo: " << info.ToString() << "\n";
    };

    std::string peerDetails;
    while (loop) {     
        serverConnection.Update(cbs);
        // Some basic UI
            // send packet
            if (_kbhit()) {
                auto c = _getch();
                if (c == '1')
                {
                    std::cout << "pressed join game 1\n";
                    if (openGames.size() > 0)
                        serverConnection.RequestToJoinGame(openGames[0]);

                }
                if (c == '2')
                {
                    std::cout << "pressed join game 2\n";
                    if (openGames.size() > 1)
                        serverConnection.RequestToJoinGame(openGames[1]);

                }
                if (c == '3')
                {
                    std::cout << "pressed join game 3\n";
                    if (openGames.size() > 2)
                        serverConnection.RequestToJoinGame(openGames[2]);

                }
                else if (c == 'c')
                {
                    std::cout << "pressed connect\n";
                    serverConnection.Connect(serverIP, 19601, name, "SimpleTestApp");
                }
                else if (c == 'd')
                {
                    std::cout << "pressed disconncet\n";
                    serverConnection.Disconnect();
                }
                else if (c == 'g')
                {
                    std::cout << "pressed create\n";
                    serverConnection.CreateGame();
                }
                else if (c == 'l')
                {
                    std::cout << "pressed leave\n";
                    serverConnection.LeaveGame();
                }
                else if (c == 'y')
                {
                    if (requestedJoiners.size()) {
                        std::cout << "pressed Yes to request to join from " << requestedJoiners[0] << "\n";
                        serverConnection.ApproveJoinRequest(requestedJoiners[0]);
                    }
                }
                else if (c == 'n')
                {
                    if (requestedJoiners.size()) {
                        std::cout << "pressed No to request to join from " << requestedJoiners[0] << "\n";
                        serverConnection.EjectPlayer(requestedJoiners[0]);
                    }
                }
                else if (c == 'k')
                {
                    if (joined.size()) {
                        std::cout << "press kick out to  " << joined[0] << "\n";
                        serverConnection.EjectPlayer(joined[0]);
                    }
                }
                else if (c == 'd')
                {
                    std::cout << "press start  " << joined[0] << "\n";
                    serverConnection.LeaveGame();
                }                
                else if (c == 'q')
                {
                    loop = false;
                }
        }
        //while (enet_host_service(client.local.get(), &event, 1) > 0)
        //{
        //    switch (event.type) {
        //    case ENET_EVENT_TYPE_CONNECT:
        //    {
        //        std::cout << "connect event, ip:  " << ToReadableString(event.peer->address) << "\n";

        //        
        //        if(contains(client.peerAddresses,event.peer->address) )
        //        {
        //            std::cout << "Connected to a Peer \n";
        //         
        //            client.peerConnections.push_back(event.peer);
        //            if (client.peerConnections.size() == 1)
        //            {
        //                holePunching = false;
        //                printf("Hole punched, use this connection\n");
        //                if (contains(client.peerCandidates, event.peer)) {
        //                    eraseAndRemove(client.peerCandidates, event.peer);
        //                    std::cout << "Peer was in our candidate list: we connected to them\n";
        //                }
        //                else
        //                    std::cout << "Peer was NOT in our candidate list: they connected to us\n";

        //                Message::Make(MessageType::Info, "hello mate my name is " + name).OnData(SendTo(event.peer));
        //            }
        //            else
        //            {
        //                printf("Redundant connection established\n");
        //            }
        //        }
        //        else
        //        {
        //            std::cout << "Connected to unknown peer...... bin it\n";
        //            enet_peer_disconnect(event.peer,0);
        //        }
        //        break;
        //    }
        //    case ENET_EVENT_TYPE_RECEIVE:
        //    {
        //        EnetPacketRAIIGuard guard(event.packet);

        //        auto msg = Message::Parse(event.packet->data, event.packet->dataLength);                  
        //        printf("We received a message: ");
        //        msg.ToConsole();
        //        if (msg.Type() == MessageType::Start)
        //        {
        //            enet_peer_reset(client.server);
        //            if (TryParseIPAddressList(msg.Content(), client.peerAddresses))
        //            {
        //                std::cout << "IPs of peers in message: ";
        //                for (const auto& peerAddress : client.peerAddresses)
        //                    std::cout << ToReadableString(peerAddress) << "\n";
        //                

        //                // initiate conncetion to peer
        //                printf("Try to connect to peers: \n");
        //                for (auto& ad : client.peerAddresses)
        //                {
        //                    client.peerCandidates.push_back(enet_host_connect(client.local.get(), &ad, 0, 0));
        //                    std::cout << ToReadableString(ad) << "\n";
        //                }
        //                std::cout << "\n";
        //            }
        //        }
        //        if (msg.Type() == MessageType::Info)
        //        {
        //            if(!strcmp(msg.Content(),"Ping"))
        //                Message::Make(MessageType::Info, "Pong").OnData(SendTo(event.peer));
        //        }

        //        break;
        //    }
        //    case ENET_EVENT_TYPE_DISCONNECT:
        //    {
        //        std::cout << "disconnect event ip " << ToReadableString(event.peer->address) << "\n";
        //        if (contains(client.peerConnections,event.peer))
        //        {
        //            if(event.peer== client.peerConnections[0])
        //                std::cout << "We Disconnected from peer: " << ToReadableString(event.peer->address) << "\n";
        //            else
        //                std::cout << "We Disconnected from redundant connection: " << ToReadableString(event.peer->address) << "\n";
        //            eraseAndRemove(client.peerConnections, event.peer);
        //        }
        //        else if (contains(client.peerCandidates, event.peer))
        //        {
        //              std::cout << "Abort connection attempt to " << ToReadableString(event.peer->address) << "\n";
        //              eraseAndRemove(client.peerCandidates, event.peer);
        //        }
        //        else
        //        {
        //              std::cout << "Disconnect from unknown\n";
        //        }
        //        break;
        //    }
        //    }
        //}



    }
}