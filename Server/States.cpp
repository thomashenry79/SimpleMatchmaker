#include "States.h"
#include "Message.h"
#include "User.h"
#include "Connections.h"
#include "Sender.h"
#include <algorithm>
#include "Utils.h"

void WatingForVersionState::ReceiveMessage(const Message& msg)
{
    if (msg.Type() != MessageType::Version)
        throw BadMessageException();

    if (m_user->TrySetVersion(msg.Content()))
        m_user->ChangeState<WatingForLocalIPState>(m_user);
}

void WatingForLocalIPState::ReceiveMessage(const Message& msg)
{
    if (msg.Type() != MessageType::Info)
        throw BadMessageException();

    if(m_user->TrySetLocaIPAddress(msg.Content()))
        m_user->ChangeState<WatingForLoginState>(m_user);
}
void WatingForLoginState::ReceiveMessage(const Message& msg)
{
    if (msg.Type() != MessageType::Login)
        throw BadMessageException();

   // std::string name = 
    std::string content = msg.Content();
    auto splitPos = content.find_first_of(':', 0);
    
    std::string name, userData;
    if (splitPos != std::string::npos)
    {
        name = content.substr(0, splitPos);
        userData = content.substr(splitPos + 1);
    }
    else
    {
        name = content;
        userData = "";
    }

    if (m_user->TrySetNameAndLogIn(name,userData))
        m_user->ChangeState<LoggedInState>(m_user);

}

void LoggedInState::ReceiveMessage(const Message& msg)
{
    if (msg.Type() == MessageType::Create)
    {
        if(m_user->CreateGame(msg.Content()))
            m_user->ChangeState<OpenedGameState>(m_user);
    }
    else if (msg.Type() == MessageType::Join)
    {
        if (m_user->RequestToJoin(msg.Content()))
        {
            m_user->ChangeState<PendingJoinState>(m_user);
        }
    }
    msg;
}

void OpenedGameState::ReceiveMessage(const Message& msg)
{
    if (msg.Type() == MessageType::Leave)
    {
        if (m_user->LeaveGame(msg.Content()))
            m_user->ChangeState<LoggedInState>(m_user);
    }
    if (msg.Type() == MessageType::Eject)
    {
        m_user->Eject(msg.Content());
    }
    if (msg.Type() == MessageType::Approve)
    {
        m_user->Approve(msg.Content());
    }
    if (msg.Type() == MessageType::Start)
    {
        m_user->TryStartGame();
    }
}
void JoinedOpenGame::ReceiveMessage(const Message& msg)
{
    if (msg.Type() == MessageType::Leave)
    {
        if (m_user->LeaveGame(msg.Content()))
            m_user->ChangeState<LoggedInState>(m_user);
    }
}

void PendingJoinState::ReceiveMessage(const Message& msg)
{
    if (msg.Type() == MessageType::Leave)
    {
        if (m_user->LeaveGame(msg.Content()))
            m_user->ChangeState<LoggedInState>(m_user);
    }
}