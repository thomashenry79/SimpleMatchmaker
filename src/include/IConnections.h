#pragma once
#include <string>
class UserChangedStateVisitor;

class IConnections {
public:
	virtual ~IConnections() = default;
	virtual UserChangedStateVisitor& UserChangeStateHandler() = 0;
	virtual bool VerifyName(const std::string& name) = 0;
	virtual bool VerifyVersion(const std::string& version) = 0;
	virtual void BroadcastMessage(const class Message& m) const = 0;;
	virtual void BroadcastActiveUsers() const = 0;
};