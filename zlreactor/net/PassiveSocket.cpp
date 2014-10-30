#include "net/PassiveSocket.h"

NAMESPACE_ZL_NET_START

PassiveSocket::PassiveSocket()
{

}

PassiveSocket::PassiveSocket(const char *ip, int port)
{
    if(!Socket::Create())
    {
        throw SocketException("Could not create server socket.");
    }
    if(!Socket::Bind(ip, port))
    {
        throw SocketException("Could not bind to port.");
    }
    if(!Socket::Listen(5))
    {
        throw SocketException("Could not listen to port.");
    }
}

PassiveSocket::~PassiveSocket()
{
}

const PassiveSocket& PassiveSocket::operator << (const std::string& data) const
{
    if(!Socket::Send(data))
    {
        throw SocketException("Could not write to socket.");
    }

    return *this;
}

const PassiveSocket& PassiveSocket::operator >> (std::string& data) const
{
    if(!Socket::Recv(data))
    {
        throw SocketException("Could not read from socket.");
    }

    return *this;
}

NAMESPACE_ZL_NET_END
