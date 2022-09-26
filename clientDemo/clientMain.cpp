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

template <typename T>
inline void hash_combine(std::size_t& seed, const T& value)
{
    seed ^= std::hash<T>()(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <class It>
inline std::size_t hash_range(It first, It last)
{
    std::size_t h = 0;
    for (; first != last; ++first)
        hash_combine(h, *first);
    return h;
}


std::vector<char> RandomBuffer(int maxSize)
{
    int bufferSize = rand() % maxSize;
    std::vector<char> buffer(bufferSize);
    buffer[0] = 0;
    for (size_t i = 1; i < bufferSize; i++)
        buffer[i] = rand() % 256;
    return buffer;
}


int main(int argc, char** argv)
{
    srand((unsigned int)time(NULL));
    //// general setting
    if (argc < 2) {
         printf("invalid command line parameters\n");
         printf("usage: Client <name>\n");
         return 0;
     }
    // set ip address and port    
   std::string name(argv[1]);
   std::string serverIP(argc >2 ? argv[2] : "77.68.32.246");
   int port = 19602;// std::stoi(argv[3]);

   // Some randome buffer representnig userdata - this can be whatever you want in swos
   auto userBlob = RandomBuffer(100);
   // take hash of blob to check that the buffer is correctly received and passed on by the server
   auto blobHash = hash_range(userBlob.begin(), userBlob.end());

   
    // init
    // -- enet
   

    std::cout << "Attempt connection to " << serverIP << ":" << port << ", with name " << name << " user data size:" << userBlob.size() <<", hash:" << blobHash%10000 <<"\n";
    EnetInitialiser enetInitGuard;
    auto logger = [](const std::string& s) {std::cout << s; };
    ServerConnection serverConnection(serverIP, port, name, userBlob.data(),userBlob.size(),"SimpleTestApp",logger);
    std::unique_ptr<P2PConnection> p2pClient(nullptr);
      

    // loop
    bool loop = true;

    std::vector<std::string> openGames;
    std::vector<std::string> requestedJoiners;
    std::vector<std::string> joined;
    

    // ***********************************************************************************************************
    // Callbacks from the server, react as required in SWOS. In this example we mainly just print 
    // to console to show the user
    ServerCallbacks cbs;
    cbs.Connected = []() {std::cout << "Connected to server. Press g to create a game. d to disconnect.\n"; };
    cbs.Disconnected = []() {std::cout << "Disconnected from server\n"; };
    cbs.Timeout = []() {std::cout << "Timeout trying to connect to server\n"; };
    cbs.JoinRequestFromOtherPlayer = [](const std::string& userName) { std::cout << userName << " Wants to join. y - allow. n - deny. l - leave game.\n"; };
    cbs.JoinRequestOK = []() {std::cout << "Requsted to join game. Waiting for host to respond. Press l to leave.\n"; };
    cbs.GameCreatedOK = []() {std::cout << "We successfully created a game. Waiting for others to join. Press l to leave game.\n"; };
    cbs.StartP2P = [&](const GameStartInfo& info) {
        std::cout << "Ready to Start game, info:\n" << info.ToString(); 
        p2pClient.reset(new P2PConnection(info, [](const std::string& s) {std::cout << s; }));
    };
    cbs.LeftGameOK = []() {std::cout << "We left the game.\n"; };
    cbs.RemovedFromGame = []() {std::cout << "We were removed from the game.\n"; };
    cbs.Approved = [](const std::string& name) {std::cout << "Approved the join request of " << name << "\n"; };
    cbs.UserList = [](const std::vector<PlayerInfo>& userNames) 
    {
        std::cout << "Active Users: ";
        for (const auto& u : userNames) {
            auto userDataHash = hash_range(u.data.begin(), u.data.end());
            std::cout << u.name << "(hash:" << userDataHash % 10000 << "), ";
        }

        std::cout << "\n";
    };
    cbs.ServerMessage = [](const std::string& msg) {std::cout << "Server Message: " << msg << "\n"; };

    cbs.OpenGames = [&](const std::vector<std::string>& games)
    {
        openGames = games;
        std::cout << "Open Games: ";
        bool isHosting = std::find(RANGE(games), name) != games.end();
        int i = 1;
        for (const auto& u : openGames)
        {
            std::cout << i << ":" << u;
            if (u == name)
                std::cout << "[You]";
            std::cout << ", ";
        }
        if (openGames.size() == 0)
        {
            std::cout << "<none>";
            std::cout << "\n";
            std::cout << "Press g to open a game\n";
        }
        else
        {
            std::cout << "\n";
            if(!isHosting)
                std::cout << "Press <number> to request to join a game\n";
        }
       
    };

    cbs.GameInfo = [&](const GameInfoStruct& info)
    {
        requestedJoiners = info.requested;
        joined = info.joined;
        std::cout << "GameInfo: " << info.ToString() << "\n";
    };


    //****************************************************************************************************************
    // Callbacks for the P2P (lobby phase). In this example we respond to usermessage by echoing back the 
    // hash of the message, so the sender can visually see it's correct
    P2PCallbacks p2pCbs;
    p2pCbs.ReceiveUserMessage = [&](const void* buffer, size_t sz)
    {
        size_t hsh = hash_range((const char*)buffer, (const char*)buffer + sz);
        std::string msg((const char*)buffer, sz);
        std::cout << "Received User Message, length " << sz << " hash: " << hsh % 10000 << "\n";
        std::cout << "Message as string:\n" << msg<<"\n";
       /* auto buf = (const char*)buffer;
        if (buf[0] == 0)
        {
            size_t hsh = hash_range((const char*)buffer, (const char*)buffer + sz);
            std::cout << "Received User Message, length " << sz << " hash: " << hsh % 10000 << "\n";
            std::vector<char> response (1 + sizeof(size_t));
            response[0] = 1;
            *(size_t*)(&response[1]) = hsh;

            p2pClient->SendUserMessage(response.data(), response.size());
        }
        else if (buf[0] == 1)
        {
            size_t* hsh = (size_t*)&buf[1];
            std::cout << "Received acknowledgement, peer thinks hash was " << (*hsh) % 10000 << "\n";
        }*/
    };

    p2pCbs.Connected = []() { std::cout << "P2PCallback:*********** Connection established ******\n"; };
    p2pCbs.Disconncted = [&]() { std::cout << "P2PCallback:*********** Connection lost ******\n"; p2pClient = nullptr; };
    p2pCbs.Timeout = [&]() { std::cout << "P2PCallback:*********** Connection attempt timed out ******\n"; p2pClient = nullptr; };
    p2pCbs.ReadyStatusChanged = []() {std::cout << "P2PCallback:************** Other player changed ready status\n"; };
    p2pCbs.StartGame = [&](const GGPOStartInfo& info) {
        std::cout << "P2PCallback:************** Ready to start GGPO Session as player " << info.yourPlayerNumber <<"\n";
        std::cout << "Use local port: " << info.yourPort<< ", ";
        std::cout << "Connect to opponent at " << info.opponentIP << ", port: " << info.opponentPort << "\n";
       
        p2pClient = nullptr;
        std::cout << "P2PConnection closed and game can start, killed p2pClient\n";
    };

   

    std::string peerDetails;
    while (loop) {    
        Sleep(1);
        serverConnection.Update(cbs);
        if (p2pClient)
        {
            p2pClient->Update(p2pCbs);
        }
        // Some basic UI
            // send packet
            if (_kbhit()) {
                auto c = _getch();
                if (c == 'q')
                {
                    loop = false;
                }

                if (p2pClient)
                {
                    if (c == 'd')
                    {
                        std::cout << "Killing p2pClient\n";
                        p2pClient = nullptr;
                    }
                    else if (c == 'p')
                    {
                        auto ping = p2pClient->GetPing();
                        std::cout << "Ping to opponent is " << ping <<"\n";
                    }
                    else if (c == 'r')
                    {
                        p2pClient->ToggleReady();
                    }
                    else if (c == 'l')
                    {
                        p2pClient = nullptr;
                    }                   
                    else if (c == 's')
                    {
                        p2pClient->TryStart();
                    }
                    else if (c == 'm')
                    {
                        auto buffer = RandomBuffer(300);
                        std::cout << "Sending a random user message, length " << buffer.size() << " hash: " << hash_range(buffer.begin(), buffer.end())%10000 <<"\n";
                        p2pClient->SendUserMessage(buffer.data(),buffer.size());
                    }
                }
                else
                {
                    if (c == '1')
                    {
                       // std::cout << "pressed join game 1\n";
                        if (openGames.size() > 0)
                            serverConnection.RequestToJoinGame(openGames[0]);

                    }
                    if (c == '2')
                    {
                       // std::cout << "pressed join game 2\n";
                        if (openGames.size() > 1)
                            serverConnection.RequestToJoinGame(openGames[1]);

                    }
                    if (c == '3')
                    {
                      //  std::cout << "pressed join game 3\n";
                        if (openGames.size() > 2)
                            serverConnection.RequestToJoinGame(openGames[2]);

                    }
                    else if (c == 'c')
                    {
                      //  std::cout << "pressed connect\n";
                        if(serverConnection.Connect(serverIP, port, name, userBlob.data(),userBlob.size(),"SimpleTestApp"))
                            std::cout << "Attempt connection to " << serverIP << ":" << port << ", with name " << name << "\n";

                    }
                    else if (c == 'd')
                    {
                      //  std::cout << "pressed disconncet\n";
                        serverConnection.Disconnect();
                    }
                    else if (c == 'g')
                    {
                      //  std::cout << "pressed create\n";
                        serverConnection.CreateGame();
                    }
                    else if (c == 'l')
                    {
                      //  std::cout << "pressed leave\n";
                        serverConnection.LeaveGame();
                    }
                    else if (c == 'y')
                    {
                        if (requestedJoiners.size()) {
                           // std::cout << "pressed Yes to request to join from " << requestedJoiners[0] << "\n";
                            serverConnection.ApproveJoinRequest(requestedJoiners[0]);
                        }
                    }
                    else if (c == 'n')
                    {
                        if (requestedJoiners.size()) {
                         //   std::cout << "pressed No to request to join from " << requestedJoiners[0] << "\n";
                            serverConnection.EjectPlayer(requestedJoiners[0]);
                        }
                    }
                    else if (c == 'k')
                    {
                        if (joined.size()) {
                         //   std::cout << "press kick out to  " << joined[0] << "\n";
                            serverConnection.EjectPlayer(joined[0]);
                        }
                    }
                }
             
        }
        



    }
}