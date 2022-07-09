#pragma once
#include "enet/enet.h"
#include <string>
std::string ToString(const ENetAddress& addr);
std::string ToReadableString(const ENetAddress& addr);