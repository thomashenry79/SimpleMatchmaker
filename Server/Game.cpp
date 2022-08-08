#include "Game.h"
#include "User.h"
#include "Utils.h"
#include <algorithm>
#include "Message.h"
#include "Sender.h"
bool Game::WasCreatedBy(const std::string& name) const
{
	return name == this->m_creator->Name();
}

bool Game::WasCreatedBy(const User* user) const
{
	return user == this->m_creator;
}
bool Game::CanStart() const
{
	return m_joined.size() == 1;
}
bool Game::RemoveJoinedOrPending( User* user)
{
	if (m_pending.count(user)) {
		m_pending.erase(user);
		SendInfoToAll();
		return true;
	}
	if (m_joined.count(user)) {
		m_joined.erase(user);
		SendInfoToAll();
		return true;
	}
	return false;
}
void Game::SendInfoToAll() const
{
	auto msg = Message::Make(MessageType::GameInfo, FullInfo());
	msg.OnData([](const std::string s) {std::cout << "Send Game info: " << s << "\n"; });
	msg.OnData(SendTo(m_creator->Peer()));
	for (auto p : m_pending)
		msg.OnData(SendTo(p->Peer()));
	for (auto p : m_joined)
		msg.OnData(SendTo(p->Peer()));
}

void Game::KillGame()
{
	auto msg = Message::Make(MessageType::Eject, m_creator->Name());

	for (auto p : m_pending) {
		p->ChangeState<LoggedInState>(p);
		msg.OnData(SendTo(p->Peer()));
	}
	for (auto p : m_joined) {
		p->ChangeState<LoggedInState>(p);
		msg.OnData(SendTo(p->Peer()));
	}
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
	SendInfoToAll();
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
	SendInfoToAll();
	return true;
}