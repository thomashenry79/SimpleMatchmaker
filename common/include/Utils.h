#pragma once
#include "enet/enet.h"
#include <string>
#include <vector>
#include <algorithm>
#include <memory>

std::string ToString(const ENetAddress& addr);

std::string ToReadableString(const ENetAddress& addr);

using ENetHostPtr = std::unique_ptr<ENetHost, void(*)(ENetHost*)>;
class EnetInitialiser
{
public:
    EnetInitialiser() : code(enet_initialize()) {}
    ~EnetInitialiser()
    {
        if (code == 0)
            enet_deinitialize();
    }
    int code;
};

class EnetPacketRAIIGuard
{
public:
    EnetPacketRAIIGuard(ENetPacket* packet) : m_packet(packet) {}
    ~EnetPacketRAIIGuard() { enet_packet_destroy(m_packet); }
    ENetPacket* m_packet;
};

template<class T>
bool contains(const std::vector<T>& v, const T& elem)
{
	return std::find(v.begin(), v.end(), elem) != v.end();
}


template<class T>
void eraseAndRemove(std::vector<T>& vec, const T& elem)
{
	vec.erase(std::remove(vec.begin(), vec.end(), elem), vec.end());
}

bool TryParseIPAddress(const std::string& msg, ENetAddress& port);

bool TryParseIPAddressList(const std::string& msg,std::vector<ENetAddress>& results);

std::vector<std::string> stringSplit(const std::string& text, char delim);