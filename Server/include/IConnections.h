#pragma once
#include <string>
#include <vector>
class IConnections {
public:
	virtual ~IConnections() = default;
	virtual bool VerifyName(const std::string& name) = 0;
	virtual bool VerifyVersion(const std::string& version) = 0;
	virtual void BroadcastMessage(const class Message& m) const = 0;;
	virtual void BroadcastActiveUsers() const = 0;
	virtual void OnUserChangeState(class User*) = 0;
	virtual bool OpenGame(User* creator, int min_players, int max_players, const std::vector<char>& data) = 0;
	virtual bool RequestToJoin(User* requestor, const std::string& data) = 0;
	virtual void BroadcastOpenGames() const = 0;
	virtual void RemoveUserFromAnyGames(User* user) = 0;
	virtual void Eject(User* owner, const std::string& other) = 0;
	virtual bool Approve(User* owner, const std::string& other) = 0;
	virtual bool StartGame(User* owner) = 0;
};