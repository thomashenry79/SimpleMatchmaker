#include "Game.h"
#include "User.h"
#include "Utils.h"
#include <algorithm>
bool Game::WasCreatedBy(const std::string& name) const
{
	return name == this->m_creator->Name();
}

bool Game::WasCreatedBy(const User* user) const
{
	return user == this->m_creator;
}

bool Game::RemoveJoinedOrPending( User* user)
{
	if (m_pending.count(user)) {
		m_pending.erase(user);
		return true;
	}
	if (m_joined.count(user)) {
		m_joined.erase(user);
		return true;
	}
	return false;
}

void Game::KillGame()
{
	for (auto p : m_pending)
		p->ChangeState<LoggedInState>(p);
	for (auto p : m_joined)
		p->ChangeState<LoggedInState>(p);
}

bool Game::Approve(const std::string& name)
{
	auto it = std::find_if(RANGE(m_pending), [&](User* u) {return u->Name() == name; });
	if (it == std::end(m_pending))
		return false;

	User* u = *it;
	m_pending.erase(it);

	u->ChangeState<JoinedOpenGame>(u);
	m_joined.insert(u);
	return true;
	
}
std::string Game::FullInfo() const
{
	std::string info;
	info += m_creator->Name() + ":";
	for (const auto& p : m_joined)
		info += p->Name() + ",";
	info +=  ":";
	for (const auto& p : m_pending)
		info += p->Name() + ",";

	return info;
}

std::string Game::ShortInfo() const
{
	return m_creator->Name();
}

bool Game::RequestUserJoin(  User* user)
{
	if (m_pending.count(user) != 0)
		return false;

	if (m_joined.count(user) != 0)
		return false;

	m_pending.insert(user);

	return true;
}