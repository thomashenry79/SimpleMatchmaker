#pragma once
class Message;
class User;

// Zombie state - user has been kicked off, can no longer do anything
struct KickedOffState
{  
    User* m_user=nullptr;
    void ReceiveMessage(const Message&){}
};

struct WatingForVersionState
{
    WatingForVersionState(User* user) : m_user(user) {}
    User* m_user;
    void ReceiveMessage(const Message& msg);
};

struct WatingForLoginState
{
    WatingForLoginState(User* user) : m_user(user) {}
    User* m_user;
    void ReceiveMessage(const Message& msg);
};

struct WatingForLocalIPState
{
    WatingForLocalIPState(User* user) : m_user(user) {}
    User* m_user;
    void ReceiveMessage(const Message& msg);
};
struct LoggedInState
{
    LoggedInState(User* user) : m_user(user) {}
    User* m_user;
    void ReceiveMessage(const Message& msg);
};

struct OpenedGameState
{
    OpenedGameState(User* user) : m_user(user) {}
    User* m_user;
    void ReceiveMessage(const Message& msg);
};

struct PendingJoinState
{
    PendingJoinState(User* user) : m_user(user) {}
    User* m_user;
    void ReceiveMessage(const Message& msg);
};


struct JoinedOpenGame
{
   JoinedOpenGame(User* user) : m_user(user) {}
   User* m_user;
   void ReceiveMessage(const Message& msg);
};
