#pragma once
#include <enet/enet.h>
#include <iostream>
struct Sender
{
    Sender(ENetPeer* to) : m_to(to) {};
    void operator()(const unsigned char* data, size_t length) {
        ENetPacket* packetPong = enet_packet_create(data, length, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(m_to, 0, packetPong);
    }
    ENetPeer* m_to;
};
