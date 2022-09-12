#include "P2PConnection.h"
#include "Message.h"
#include "Sender.h"
using namespace std::chrono;
P2PConnection::P2PConnection(GameStartInfo info, std::function<void(const std::string&)> logger) :
    m_info(info),
    localAddress{ ENET_HOST_ANY,info.port },
    local(enet_host_create(&localAddress, info.peerAddresses.size()*2, 0, 0, 0),
        enet_host_destroy),
    m_logger(logger)
{
    for(auto& address : m_info.peerAddresses)
        outGoingPeerCandidates.push_back(enet_host_connect(local.get(), &address, 0, 0));

    logger("********Start P2P connection process with " + info.peerName + ". Try connections on: \n");
    for (const auto& addr : m_info.peerAddresses)
        logger(ToReadableString(addr) + "\n");
    logger("****************\n");
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

void P2PConnection::SendStart()
{
    if (m_info.playerNumber != 1) {
        m_logger("Only player 1 can start the game\n");
        return;
    }
    if (!(m_bMeReady && m_bOtherReady))
    {
        m_logger("Cannot start yet, both players are not ready\n");
        return;
    }
    if (m_Start)
    {
        m_logger("Already initiated Start process\n");
        return;
    }
    if (!peerConnections.size())
    {
        m_logger("No peer connection\n");
        return;
    }
    if (!m_bPrimaryConnectionEstablished)
    {
        m_logger("No primary connection agreed\n");
        return;
    }
    
     Message::Make(MessageType::Info, "Start").OnData(SendTo(peerConnections[0]));
     m_Start = true;
     m_logger("Initiating Start process: Sent Start message, then disconnect\n");
     for(auto& peer : peerConnections)
         enet_peer_disconnect_later(peer, 0);
    
}

void P2PConnection::SendReady()
{
    if (m_bMeReady)
        return;

    if (peerConnections.size())
    {
        Message::Make(MessageType::Info, "Ready").OnData(SendTo(peerConnections[0]));
        m_bMeReady = true;
        OnReadyChange();
    }
}

void P2PConnection::OnReadyChange()
{
    if (m_bMeReady)
        m_logger("I'm ready, ");
    else
        m_logger("I'm not ready, ");

    if (m_bOtherReady)
        m_logger("other player is ready\n");
    else
        m_logger("other player is not ready\n");

    if (m_bMeReady && m_bOtherReady)
        m_logger(m_info.playerNumber == 1 ? "Press S to start game\n" : "Waiting for player 1 to start game\n");
        
}
bool P2PConnection::ReadyToStart() const
{
    bool allConnectionsDead = peerConnections.size() + outGoingPeerCandidates.size() == 0;
    bool ready = m_Start && allConnectionsDead;
    return ready;
} 

void P2PConnection::Info()
{
    m_logger( "**************Lobby Connection Info:***********\n");
    m_logger(std::string("Number of active connections : ") + std::to_string(peerConnections.size()) +"\n");
    m_logger(std::string("Number of pending connections : ")+ std::to_string(outGoingPeerCandidates.size()) + "\n");
    if (m_bPrimaryConnectionEstablished && peerConnections.size())
        m_logger(std::string("Primary connection is to :") + ToReadableString(peerConnections[0]->address) + "\n");
    m_logger("*********\n");
}

void P2PConnection::CleanRedundantConnections()
{
    if ( m_bPrimaryConnectionEstablished && TotalActivePeers()>1)
    {
        for (size_t i = 1; i < peerConnections.size(); i++)
                enet_peer_reset(peerConnections[i]);

        eraseAndRemoveIfNot(peerConnections, peerConnections[0]);

        for (size_t i = 0; i < outGoingPeerCandidates.size(); i++)
            enet_peer_reset(outGoingPeerCandidates[i]);

        outGoingPeerCandidates.clear();
        Info();
    }
}


PingHandler::PingHandler() : lastPing(high_resolution_clock::now())
{

}

void PingHandler::Update(ENetPeer* peerToPing)
{
    if (m_bPingSent)
        return;

    if (!peerToPing)
        return;

    auto now = high_resolution_clock::now();
    auto timeSinceLastPing = (int)duration_cast<milliseconds>(now - lastPing).count();
    if (timeSinceLastPing >= pingPeriodMS)
    {
        Message::Make(MessageType::Info, "Ping").OnData(SendTo(peerToPing));
        lastPing = now;
        m_bPingSent = true;
    }
}
//#include <iostream>
void PingHandler::OnPong()
{
    auto now = high_resolution_clock::now();
    auto thisPing = (double)duration_cast<microseconds>(now - lastPing).count();
    
    //ignore silly big pings, these are probablt not representative
    if (thisPing > 2 *1e6)
        return;

    if (m_pingEMA == 0)
        m_pingEMA = thisPing;
    
    m_pingEMA = ((thisPing - m_pingEMA) * EMA_Constant) + m_pingEMA;
   // std::cout << "last ping was " << thisPing << "us" << ", ping average: " << m_pingEMA <<"us\n";
    
    m_bPingSent = false;
}

double P2PConnection::GetPing() const 
{
    return m_pingHandler.GetPing()/1e3;
}

void P2PConnection::SendUserMessage(char* buffer, size_t length)
{
    if (m_bPrimaryConnectionEstablished && peerConnections.size() == 1)
        Message::Make(MessageType::UserMessage, std::string(buffer,length)).OnData(SendTo(peerConnections[0]));
}
void P2PConnection::Update(P2PCallbacks& callbacks)
{
    ENetEvent event;
    CleanRedundantConnections();
    if (m_bPrimaryConnectionEstablished && peerConnections.size() == 1)
        m_pingHandler.Update(peerConnections[0]);

    while (enet_host_service(local.get(), &event, 1) > 0)
        {
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
            {
                m_logger(std::string("connect event, ip: ") + ToReadableString(event.peer->address) + " id: " + std::to_string(event.peer->connectID) + "\n");

                // If the incoming connection is from the candidate list, accept it and clear the candidate list
                if(contains(m_info.peerAddresses,event.peer->address))
                {
                    eraseAndRemoveIfNot(m_info.peerAddresses, event.peer->address);
                    m_logger("Connected to a Peer \n");
                 
                    peerConnections.push_back(event.peer);

                    if (contains(outGoingPeerCandidates, event.peer)) {
                        eraseAndRemove(outGoingPeerCandidates, event.peer);
                        m_logger("Peer was in our candidate list: we connected to them\n");
                    }
                    else
                        m_logger("Peer was not in our outgoing list: they connected to us\n");

                    // Is this the first connection?
                    if (peerConnections.size() == 1)
                    {   
                        // If we are player1, we tell the other player to use this connction as the primary one
                        if (m_info.playerNumber == 1)
                        {
                            m_bPrimaryConnectionEstablished = true;
                            Message::Make(MessageType::Info, "PRIMARY").OnData(SendTo(event.peer));
                        }
                    }
                    else
                    {
                        m_logger("Redundant connection established\n");
                    }
                }
                else
                {
                    m_logger(std::string("Incoming connection from unexpected peer: ") + ToReadableString(event.peer->address) + " bin it\n");
                    enet_peer_reset(event.peer);
                }
                Info();
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE:
            {
                EnetPacketRAIIGuard guard(event.packet);
                

                auto msg = Message::Parse(event.packet->data, event.packet->dataLength);                  
               
                if (msg.Type() == MessageType::UserMessage)
                {
                    msg.OnPayload(callbacks.ReceiveUserMessage);
                  //  callbacks.RecieveMessage(msg.Content().data)
                }
                else if (msg.Type() == MessageType::Info)
                {
                    if (strcmp(msg.Content(), "Ping") != 0 && strcmp(msg.Content(), "Pong"))
                    {
                        m_logger("We received a message: ");
                        msg.ToConsole();
                    }

                    if (!strcmp(msg.Content(), "PRIMARY"))
                    {
                        m_bPrimaryConnectionEstablished = true;
                        if (peerConnections.size() && (peerConnections[0] == event.peer))
                        {
                            m_logger("Agree with player 1 on primary connection\n");
                        }
                        else 
                        {
                            auto it = std::find(RANGE(peerConnections), event.peer);
                            if (it == peerConnections.end())
                                m_logger("Player 1's primary connection is not even present for us.. this should never happen\n");
                            else
                            {
                                std::iter_swap(peerConnections.begin(), it);
                                m_logger("Did't agree with player 1 on primary connection, swapped ours to match.\n");
                            }
                        }
                    }
                    if(!strcmp(msg.Content(),"Ping"))
                        Message::Make(MessageType::Info, "Pong").OnData(SendTo(event.peer));
                    
                    if (!strcmp(msg.Content(), "Pong"))
                    {
                        m_pingHandler.OnPong();
                    }
                    if (!strcmp(msg.Content(), "Start"))
                    {
                        if(m_Start)
                            m_logger("Other player sent duplicate Start message\n");
                        else
                        {
                            m_Start = true;
                            m_logger("Other player intiated Start Process, expect disconnect soon\n");
                        }
                    }
                    if (!strcmp(msg.Content(), "Ready"))
                    {
                        m_bOtherReady = true;
                        OnReadyChange();
                    }
                }

                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT:
            {
                m_logger(std::string("disconnect event ip ") + ToReadableString(event.peer->address) + "\n");
                if (contains(peerConnections,event.peer))
                {
                    if(event.peer== peerConnections[0])
                        m_logger(std::string("We Disconnected from primary connection: ")+ ToReadableString(event.peer->address) + "\n");
                    else
                        m_logger(std::string("We Disconnected from redundant connection: ") + ToReadableString(event.peer->address) + "\n");
                    eraseAndRemove(peerConnections, event.peer);
                }
                else if (contains(outGoingPeerCandidates, event.peer))
                {
                    m_logger(std::string("Abort connection attempt to ") + ToReadableString(event.peer->address) + "\n");
                    eraseAndRemove(outGoingPeerCandidates, event.peer);
                }
                else
                {
                    m_logger("Disconnect from unknown\n");
                }
                Info();
                if (ReadyToStart())
                {
                    // We don't do anything after this                
                    m_logger("All connections closed, Start is set, let's go!!!\n");
                }
               
                break;
            }
            }
        }
}

