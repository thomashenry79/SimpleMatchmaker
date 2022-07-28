#pragma once
#include <vector>
#include <string>
#include <set>
class User;

class Game
{
public:
	Game(User* creator, int minPlayers, int maxPlayers) : m_creator(creator), m_minPlayers(minPlayers),m_maxPlayers(maxPlayers) {};
	void KillGame();
	bool WasCreatedBy(const std::string& name) const;
	 User* CreatedBy() const { return m_creator; }
	bool WasCreatedBy(const User* user) const;
	void RemoveJoinedOrPending( User* user) ;
	bool RequestUserJoin(User* user);
	bool Approve(const std::string& name);
	std::string FullInfo() const;
	std::string ShortInfo() const;
private:
	User* m_creator;
	std::set< User*> m_joined;
	std::set< User*> m_pending;
	int m_maxPlayers;
	int m_minPlayers;
};