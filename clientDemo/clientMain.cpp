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
#include "P2PConnection.h"
#include "Sender.h"







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
    std::unique_ptr<P2PConnection> p2pClient(nullptr);
      

    // loop
    bool loop = true;
    int pongs = 0;
    int loopCount = 0;
  
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
    cbs.StartP2P = [&](const GameStartInfo& i) {
        std::cout << "Ready to Start game, info:\n" << i.ToString(); 
        p2pClient.reset(new P2PConnection(i)); 
    };
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
        if (p2pClient)
            p2pClient->Update();
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
                else if (c == 'p')
                {
                    if (p2pClient)
                        p2pClient->SendPing();
                }
        }
        



    }
}