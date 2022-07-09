#include "Utils.h"

std::string ToString(const ENetAddress& addr)
{
    return std::to_string(addr.host) + ":" + std::to_string(addr.port);
}