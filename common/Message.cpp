#include "Message.h"
#include <iostream>

const std::map< MessageType, std::string> Message::headers = std::map< MessageType, std::string>{
    {MessageType::Info,"INFO:" },
    {MessageType::Version, "VERSION:"},
    {MessageType::Login,"LOGIN:" },
    {MessageType::Create,"CREATE:" },
    {MessageType::Join,"JOIN:" },
    {MessageType::Leave,"LEAVE:" },
    {MessageType::Eject,"EJECT:" },
    {MessageType::Start,"START:" }
};

void Message::OnData(std::function<void(const std::string&)> callback) const
{
    callback(m_data);
}

Message Message::Make(MessageType type, std::string content)
{
    return Message(type, headers.at(type) + content);
}

 Message Message::Parse(const unsigned char* data, size_t len)
{
    std::string input(data, data + len);
    auto it = std::find_if(headers.begin(), headers.end(), [&](const auto& header) {
        auto s = header.second;
        return input.compare(0, s.size(), s) == 0;
        });
    
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


void Message::ToConsole() const
{
    std::cout << m_data << "\n";
}