#pragma once
#include <functional>
#include <string>
#include <map>
#include <tuple>
#include <cstdint>
enum class MessageType
{
    Info,
    Login,
    Version,
    Create,
    Join,
    Leave,
    Eject,
    Start,
    PlayersActive,
    GamesOpen
};

class BadMessageException : public std::exception {};

class Message
{
public:
    void OnData(std::function<void(const std::string&)> callback) const;    
    static Message Make(MessageType type, std::string content);
    static Message Parse(const unsigned char* data, size_t len);
    MessageType Type() const;
    const char* Content() const;
    void ToConsole() const;
private:
    Message(MessageType type, std::string data) : m_type(type), m_data(data) {};
    MessageType m_type;
    std::string m_data;
    static const std::map< MessageType, std::string> headers;
};

