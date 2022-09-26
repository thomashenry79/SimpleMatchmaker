#pragma once
#include "enet/enet.h"
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <memory>

#define RANGE(x) std::begin(x), std::end(x)
std::string ToString(const ENetAddress& addr);


bool operator==(const ENetAddress& lhs, const ENetAddress& rhs);
bool operator!=(const ENetAddress& lhs, const ENetAddress& rhs);
std::string ToReadableString(const ENetAddress& addr);
std::string ToReadableIPv4String(const ENetAddress& addr);
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


template<class C, class T>
void eraseAndRemove(C& container, const T& elem)
{
	container.erase(std::remove(std::begin(container), std::end(container), elem), std::end(container));
}
template<class C, class T>
void eraseAndRemoveIfNot(C& container, const T& elem)
{
    container.erase(std::remove_if(std::begin(container), std::end(container), [&](const T& el) {return el != elem; }), std::end(container));
}
bool TryParseIPAddress(const std::string& msg, ENetAddress& port);

bool TryParseIPAddressList(const std::string& msg,std::vector<ENetAddress>& results);

std::vector<std::string> stringSplit(const std::string& text, char delim);