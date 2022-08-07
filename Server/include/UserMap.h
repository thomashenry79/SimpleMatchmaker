#pragma once
#include <enet/enet.h>
#include <map>
#include <memory>
#include "User.h"

using UserMap = std::map<ENetPeer*, std::unique_ptr<User>>;

template <class Visitor>
Visitor& VisitMap(const UserMap& users, Visitor& v)
{
    for (auto& pair : users)
        pair.second->Visit(v);
    return v;
}