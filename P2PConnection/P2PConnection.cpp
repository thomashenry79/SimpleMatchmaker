#include "P2PConnection.h"
#include <iostream>
#include "Message.h"
#include "Sender.h"
using namespace std::chrono;
P2PConnection::P2PConnection(GameStartInfo info) :
    m_info(info),
    localAddress{ ENET_HOST_ANY,info.port },
    local(enet_host_create(&localAddress, 4, 0, 0, 0),
        enet_host_destroy)
{
    for(auto& address : m_info.peerAddresses)
        outGoingPeerCandidates.push_back(enet_host_connect(local.get(), &address, 0, 0));
}

P2PConnection::~P2PConnection()
{
    if (local)
    {
        for (auto& peer : peerConnections)
            enet_peer_disconnect(peer,0);

        enet_host_flush(local.get());
    }
}


void P2PConnection::SendPing() 
{
    if (local && peerConnections.size())
    {
        Message::Make(MessageType::Info, "Ping").OnData(SendTo(peerConnections[0]));
        lastPing = high_resolution_clock::now();
    }
}
void P2PConnection::SendStart()
{
    if (m_info.playerNumber != 1) {
        std::cout << "Only player 1 can start the game\n";
        return;
    }
    if (!(m_bMeReady && m_bOtherReady))
    {
        std::cout << "Cannot start yet, both players are not ready\n";
        return;
    }
    if (m_Start)
    {
        std::cout << "Already initiated Start process\n";
        return;
    }
    if (local && peerConnections.size())
    {
        Message::Make(MessageType::Info, "Start").OnData(SendTo(peerConnections[0]));
        m_Start = true;
        std::cout << "Initiating Start process: Sent Start message, then disconnect\n";
        for(auto& peer : peerConnections)
            enet_peer_disconnect_later(peer, 0);
    }
}

void P2PConnection::SendReady()
{
    if (m_bMeReady)
    {
        std::cout << "You are already ready\n";
        return;
    }
    if (local && peerConnections.size())
    {
        Message::Make(MessageType::Info, "Ready").OnData(SendTo(peerConnections[0]));
        m_bMeReady = true;
        OnReadyChange();
    }
}

void P2PConnection::OnReadyChange()
{
    if (m_bMeReady)
        std::cout << "I'm ready, ";
    else
        std::cout << "I'm not ready, ";

    if (m_bOtherReady)
        std::cout << "other player is ready\n";
    else
        std::cout << "other player is not ready\n";

    if (m_bMeReady && m_bOtherReady)
        SendStart();
}
bool P2PConnection::ReadyToStart() const
{
    bool allConnectionsDead = peerConnections.size() == 0;
    bool ready = m_Start && allConnectionsDead;
    return ready;
} 
void P2PConnection::Update()
{
    ENetEvent event;
    while (enet_host_service(local.get(), &event, 1) > 0)
        {
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
            {
                std::cout << "connect event, ip:  " << ToReadableString(event.peer->address) << "\n";

                // If the incoming connection is from the candidate list, accept it and clear the candidate list
                if(contains(m_info.peerAddresses,event.peer->address) )
                {
                    eraseAndRemoveIfNot(m_info.peerAddresses, event.peer->address);
                    std::cout << "Connected to a Peer \n";
                 
                    peerConnections.push_back(event.peer);
                    if (peerConnections.size() == 1)
                    {
                        printf("Use this connection\n");
                        if (contains(outGoingPeerCandidates, event.peer)) {
                            eraseAndRemove(outGoingPeerCandidates, event.peer);
                            std::cout << "Peer was in our candidate list: we connected to them\n";
                        }
                        else
                            std::cout << "Peer was NOT in our candidate list: they connected to us\n";

                        Message::Make(MessageType::Info, "hello mate my name is " + m_info.yourName).OnData(SendTo(event.peer));
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
                EnetPacketRAIIGuard guard(event.packet);

                auto msg = Message::Parse(event.packet->data, event.packet->dataLength);                  
                printf("We received a message: ");
                msg.ToConsole();
                
                if (msg.Type() == MessageType::Info)
                {
                    if(!strcmp(msg.Content(),"Ping"))
                        Message::Make(MessageType::Info, "Pong").OnData(SendTo(event.peer));
                    
                    if (!strcmp(msg.Content(), "Pong"))
                    {
                        auto pingTime = (int)duration_cast<microseconds>(high_resolution_clock::now() - lastPing).count();
                        std::cout << "Ping time was " << pingTime << "us\n";
                    }
                    if (!strcmp(msg.Content(), "Start"))
                    {
                        if(m_Start)
                            std::cout << "Other player sent duplicate Start message\n";
                        else
                        {
                            m_Start = true;
                            std::cout << "Other player intiated Start Process, expect disconnect\n";
                        }
                    }
                    if (!strcmp(msg.Content(), "Ready"))
                    {
                        if (m_bOtherReady)
                        {
                            std::cout << "Other player sent duplicate Ready message\n";
                        }
                        else
                        {
                            m_bOtherReady = true;
                            OnReadyChange();
                        }
                        
                    }
                }

                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT:
            {
                std::cout << "disconnect event ip " << ToReadableString(event.peer->address) << "\n";
                if (contains(peerConnections,event.peer))
                {
                    if(event.peer== peerConnections[0])
                        std::cout << "We Disconnected from peer: " << ToReadableString(event.peer->address) << "\n";
                    else
                        std::cout << "We Disconnected from redundant connection: " << ToReadableString(event.peer->address) << "\n";
                    eraseAndRemove(peerConnections, event.peer);
                }
                else if (contains(outGoingPeerCandidates, event.peer))
                {
                      std::cout << "Abort connection attempt to " << ToReadableString(event.peer->address) << "\n";
                      eraseAndRemove(outGoingPeerCandidates, event.peer);
                }
                else
                {
                      std::cout << "Disconnect from unknown\n";
                }
                if (ReadyToStart())
                {
                    // We don't do anything after this                
                    std::cout << "All connections closed, Start is set, let's go!!!\n";
                }
                break;
            }
            }
        }
}

