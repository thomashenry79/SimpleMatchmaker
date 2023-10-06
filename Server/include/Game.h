#pragma once
#include <vector>
#include <string>
#include <set>
class User;

class Game
{
public:
	Game(User* creator, int minPlayers, int maxPlayers,const std::vector<char>& data) : m_creator(creator), m_minPlayers(minPlayers),m_maxPlayers(maxPlayers),m_data(data) {};
	void KillGame();
	bool WasCreatedBy(const std::string& name) const;
	 User* CreatedBy() const { return m_creator; }
	bool WasCreatedBy(const User* user) const;
	bool RemoveJoinedOrPending( User* user) ;
	bool RequestUserJoin(User* user);
	bool Approve(const std::string& name);
	void SendInfoToAll() const;
	bool CanStart() const;
	std::string ShortInfo() const;
	User* FirstJoiner() { return *m_joined.begin(); }
	const std::vector<char>& Data() const { return m_data; }
private:
	std::string FullInfo() const;
	User* m_creator;
	std::set< User*> m_joined;
	std::set< User*> m_pending;
	int m_maxPlayers;
	int m_minPlayers;
	std::vector<char> m_data;
};