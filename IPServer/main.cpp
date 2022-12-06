#include <stdio.h>
#include <enet/enet.h>
#include <stdint.h>
#include <chrono>
#include <string>
#include <map>
#include <vector>
#include "Message.h"
#include "Sender.h"
#include <iostream>
#include "Utils.h"

int main(int argc, char** argv)
{
    if (argc < 2) {
        printf("invalid command line parameters\n");
        printf("usage: IPServer <port>\n");
        return 0;
    }
    // set ip address and port   
    uint16_t port =  std::stoi(argv[1]);
    EnetInitialiser enetObj;
    ENetAddress address{ ENET_HOST_ANY,port };
    ENetHostPtr host(enet_host_create(&address, ENET_PROTOCOL_MAXIMUM_PEER_ID, 0, 0, 0), enet_host_destroy);
    // loop
    bool loop = true;
    std::cout << "IP Server v4 running on Port " << port << "\n";
    while (loop) {
        ENetEvent event;
#ifdef WIN32
            Sleep(1);
#else
            sleep(1);
#endif // WIN32
        while (enet_host_service(host.get(), &event, 0))
        {  
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
            {
                auto incomingIP = ToString(event.peer->address);
                std::cout << "Connection from  " << incomingIP << "\n";
                Message::Make(MessageType::Info, incomingIP).OnData(SendTo(event.peer));
                enet_peer_disconnect_later(event.peer, 0);
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE:
            {
                EnetPacketRAIIGuard guard(event.packet);
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT:
            {
                enet_peer_reset(event.peer);
                break;
            }
            case ENET_EVENT_TYPE_NONE:
                break;
            }
        }
    }

}