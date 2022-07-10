#pragma once
#include "enet/enet.h"
#include <string>
#include <vector>
#include <algorithm>
std::string ToString(const ENetAddress& addr);
std::string ToReadableString(const ENetAddress& addr);

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