#include "Utils.h"

std::string ToString(const ENetAddress& addr)
{
    return std::to_string(addr.host) + ":" + std::to_string(addr.port);
}

std::string ToReadableString(const ENetAddress& addr)
{
    char fromIP[40];
    enet_address_get_host_ip(&addr, fromIP, 40);
    return std::string(fromIP) + ":" + std::to_string(addr.port);
}