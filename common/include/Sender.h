#pragma once
#include <enet/enet.h>
#include <iostream>
#include <string>
struct Sender
{
    Sender(ENetPeer* to) : m_to(to) {};
    void operator()(const std::string& s) {
        ENetPacket* packetPong = enet_packet_create(s.c_str(), s.length(), ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(m_to, 0, packetPong);
    }
    ENetPeer* m_to;
};
