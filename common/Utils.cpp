#include "Utils.h"
#include <sstream>
std::string ToString(const ENetAddress& addr)
{
    return std::to_string(addr.host) + ":" + std::to_string(addr.port);
}
bool operator==(const ENetAddress& lhs, const ENetAddress& rhs)
{
    return lhs.host == rhs.host && lhs.port == rhs.port;
}
bool operator!=(const ENetAddress& lhs, const ENetAddress& rhs)
{
    return !(lhs == rhs);
}
std::string ToReadableString(const ENetAddress& addr)
{
    char fromIP[40];
    enet_address_get_host_ip(&addr, fromIP, 40);
    return std::string(fromIP) + ":" + std::to_string(addr.port);
}

std::string ToReadableIPv4String(const ENetAddress& addr)
{
    char fromIP[40];
    enet_address_get_host_ip(&addr, fromIP, 40);
    return std::string(fromIP);
}
bool TryParseIPAddressList(const std::string& msg, std::vector<ENetAddress>& results)
{
    auto strings = stringSplit(msg, ',');
    for (const auto& s : strings)
    {
        ENetAddress addr;
        if (TryParseIPAddress(s, addr))
            results.push_back(addr);
    }
    return results.size() > 0;

}
bool TryParseIPAddress(const std::string& msg, ENetAddress& addr)
{
    try
    {
     auto strings = stringSplit(msg, ':');

     if (strings.size() != 2)
        return false;

    
        addr.host = std::stoul(strings[0]);
        addr.port = std::stoi(strings[1]);
        return true;
    }
    catch (...)
    {
        return false;
    }

}

std::vector<std::string> stringSplit(const std::string& text, char delim) {
    std::string line;
    std::vector<std::string> vec;
    std::stringstream ss(text);
    while (std::getline(ss, line, delim)) {
        vec.push_back(line);
    }
    return vec;
}
