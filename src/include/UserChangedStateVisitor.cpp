#include "UserChangedStateVisitor.h"
#include "Message.h"
#include "Sender.h"
#include "User.h"




void UserChangedStateVisitor::Visit(const JoinedOpenGame& s)
{
}

void UserChangedStateVisitor::Visit(const OpenedGameState& s)
{
}

// a user became logged in
void UserChangedStateVisitor::Visit(const LoggedInState& s)
{
	Message::Make(MessageType::Login, s.m_user->Name()).OnData(SendTo(s.m_user->Peer()));    
	m_connections.BroadcastMessage(Message::Make(MessageType::Info, "Player connected"));
    m_connections.BroadcastActiveUsers();
}

void UserChangedStateVisitor::Visit(const WatingForLocalIPState& s)
{
}

void UserChangedStateVisitor::Visit(const WatingForLoginState& s)
{
}

void UserChangedStateVisitor::Visit(const WatingForVersionState& s)
{
}

void UserChangedStateVisitor::Visit(const KickedOffState& s)
{
	// shutdown any open game this player has, remove them from any they are in
}