#pragma once
#include <enet/enet.h>
#include <iostream>
#include <string>
#include <vector>
struct SendTo
{
    SendTo(ENetPeer* to) : m_tos({ to }) {};
    SendTo(std::vector<ENetPeer*>& tos) : m_tos(tos) {};

    void operator()(const std::string& s) {
        auto packet= enet_packet_create(s.c_str(), s.length(), ENET_PACKET_FLAG_RELIABLE);
        for(auto& to : m_tos)
            enet_peer_send(to, 0, packet);
    }
    std::vector<ENetPeer*> m_tos;
};
