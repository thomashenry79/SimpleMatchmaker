#include "States.h"
#include "Message.h"
#include "User.h"
#include "Connections.h"
#include "Sender.h"
#include <algorithm>
void AwaitingVerificatonState::ReceiveMessage(const Message& msg)
{
    if (msg.Type() != MessageType::Login)
        throw BadMessageException();

    std::string name = msg.Content();

    // Remove comma so this can be used as a delimiter
    name.erase(std::remove(name.begin(), name.end(), ','), name.end());

    if (m_user->TrySetName(name))
    {
        m_user->ChangeState<LoggedInState>(m_user);
    }
}

void LoggedInState::ReceiveMessage(const Message& msg)
{
    msg;
}

void OpenedGameState::ReceiveMessage(const Message& msg)
{
    msg;
}
void JoinedOpenGame::ReceiveMessage(const Message& msg)
{
    msg;
}