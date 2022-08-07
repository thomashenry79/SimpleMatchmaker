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
#include "Utils.h"

int main(int argc, char** argv)
{
    EnetInitialiser enetObj;
    int port = 19601;
    Connections connections(port);
    // loop
    bool loop = true; 
    std::cout << "SimpleMatchmaking Server running on Port " << port << "\n";
    while (loop) {
    ENetEvent event;
        connections.Update();
        while (enet_host_service(connections.Host(), &event, 1) > 0)
        {
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
            { 
                connections.NewConnection(event.peer);                    
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE:
            {
                EnetPacketRAIIGuard guard(event.packet);                
                connections.ReceiveMessage(event.peer, event.packet->data, event.packet->dataLength);  
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT:
            {
                connections.LostConnection(event.peer);
                break;
            }
            case ENET_EVENT_TYPE_NONE:
                break;
            }
        }
    }

}