#include "Message.h"
#include <iostream>




const  std::map< MessageType, std::string> Message::headers = std::map< MessageType, std::string>{
    {MessageType::Info,"INFO:" },
    {MessageType::Login,"LOGIN:" },
    {MessageType::Version, "VERSION:"},   
    {MessageType::Create,"CREATE:" },
    {MessageType::Join,"JOIN:" },
    {MessageType::Leave,"LEAVE:" },
    {MessageType::Eject,"EJECT:" },
    {MessageType::Approve,"APPROVE:" },
    {MessageType::Start,"START:" },
    {MessageType::PlayersActive,"PLAYERSACTIVE:" },
    {MessageType::GamesOpen,"GAMESOPEN:" },
    {MessageType::GameInfo,"GAMEINFO:" },
    {MessageType::UserMessage,"USERMESSAGE:" },
    {MessageType::Ready,"READY:" },
};

void Message::OnData(std::function<void(const std::string&)> callback) const
{
    callback(m_data);
}
void Message::OnPayload(std::function<void(const void*, size_t)> callback) const
{
    callback((const void*)Content(), m_data.size() - headers.at(m_type).length());
}
Message Message::Make(MessageType type, std::string content)
{
    return Message(type, headers.at(type) + content);
}

 Message Message::Parse(const unsigned char* data, size_t len)
{
    std::string input(data, data + len);
    std::map< MessageType, std::string>::const_iterator it = headers.begin();
    for (; it != headers.end();it++)
    {
        auto s = it->second;
        if (input.compare(0, s.size(), s) == 0)
            break;
    }
    
    if (it == headers.end())
        throw BadMessageException();

     return Message(it->first, input);
   
}
MessageType Message::Type() const {
    return m_type;
}
const char* Message::Content() const
{
    return m_data.c_str() + headers.at(m_type).length();
}
//
//bool Message::TryParseIPAddress(uint32_t& addr, uint16_t& port) const
//{
//    if (m_type != MessageType::Start)
//        return false;
//
  //  auto strings = split(Content(), ':');
//
//    if (strings.size() != 2)
//        return false;
//
//    try
//    {
//        addr = std::stoul(strings[0]);
//        port = std::stoi(strings[1]);
//        return true;
//    }
//    catch(...)
//    {
//        return false;
//    }
//
//}

void Message::ToConsole() const
{
    std::cout << m_data << "\n";
}