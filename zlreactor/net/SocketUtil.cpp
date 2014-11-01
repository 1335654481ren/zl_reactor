﻿#include "net/SocketUtil.h"
#include <string.h>
NAMESPACE_ZL_NET_START

SocketInitialization  g_socket_init_once;

int socketInitialise()
{
#ifdef OS_WINDOWS
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    return 0;
}

int socketCleanup()
{
#ifdef OS_WINDOWS
    return WSACleanup();
#endif
    return 0;
}

ZL_SOCKET createSocket()
{
    ZL_SOCKET sockfd = ZL_CREATE_SOCKET(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    return sockfd;
}

ZL_SOCKET socketCreateAndListen(const char *ip, int port, int backlog)
{
    ZL_SOCKET sockfd = ZL_CREATE_SOCKET(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sockfd < 0) return sockfd;
    setNonBlocking(sockfd, true);

    ZL_SOCKADDR_IN  sockaddr;
    ::memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    int nIP = 0;
    if(!ip || '\0' == *ip || 0 == strcmp(ip, "0")
        || 0 == strcmp(ip, "0.0.0.0") || 0 == strcmp(ip, "*"))
    {
        nIP = htonl(INADDR_ANY);
    }
    else
    {
        nIP = inet_addr(ip);
    }
    sockaddr.sin_addr.s_addr = nIP;

    int res = ZL_BIND(sockfd, (struct sockaddr *) &sockaddr, sizeof(sockaddr));
    if(res < 0) return res;

    res = ZL_LISTEN(sockfd, backlog);
    if(res < 0) return res;

    return sockfd;
}

ZL_SOCKET acceptOne(ZL_SOCKET sockfd, ZL_SOCKADDR_IN *addr)
{
    int addrlen = sizeof(*addr);
    ZL_SOCKET connfd = ZL_ACCEPT(sockfd, (sockaddr *)addr, (socklen_t *)&addrlen);
    return connfd;
}

int setNonBlocking(ZL_SOCKET fd, bool nonBlocking/* = true*/)
{
#if defined(OS_LINUX)
    int flags = fcntl(fd, F_GETFL);
    if(flags < 0)
        return flags;

    nonBlocking ? flags |= O_NONBLOCK : flags &= (~O_NONBLOCK);
    return fcntl(fd, F_SETFL, flags);
#elif defined(OS_WINDOWS)
    unsigned long ul = nonBlocking ? 1 : 0;
    int ret = ioctlsocket(fd, FIONBIO, (unsigned long *)&ul);
    return ret;
#endif
}

int setNoDelay(ZL_SOCKET fd, bool noDelay/* = true*/)
{
#ifdef OS_LINUX
    int optval = noDelay ? 1 : 0;
#elif defined(OS_WINDOWS)
    bool optval = noDelay;
#endif
    return ZL_SETSOCKOPT(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&optval, sizeof(optval));
}

int setSocketReadSize(ZL_SOCKET fd, int readSize)
{
    return ZL_SETSOCKOPT(fd, SOL_SOCKET, SO_SNDBUF, &readSize, sizeof(readSize));
}

int setSocketWriteSize(ZL_SOCKET fd, int writeSize)
{
    return ZL_SETSOCKOPT(fd, SOL_SOCKET, SO_RCVBUF, &writeSize, sizeof(writeSize));
}

typedef struct sockaddr SA;
const SA* sockaddr_cast(const struct sockaddr_in* addr)
{
    return static_cast<const SA*>((const void*)addr);
}
SA* sockaddr_cast(struct sockaddr_in* addr)
{
    return static_cast<SA*>((void*)addr);
}

struct sockaddr_in getLocalAddr(ZL_SOCKET sockfd)
{
    struct sockaddr_in localaddr;
    bzero(&localaddr, sizeof localaddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
    if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0)
    {
        printf("sockets::getLocalAddr\"");
    }
    return localaddr;
}

struct sockaddr_in getPeerAddr(ZL_SOCKET sockfd)
{
    struct sockaddr_in peeraddr;
    bzero(&peeraddr, sizeof peeraddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
    if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0)
    {
        printf("sockets::getPeerAddr\"");
    }
    return peeraddr;
}

bool isSelfConnect(ZL_SOCKET sockfd)
{
    struct sockaddr_in localaddr = getLocalAddr(sockfd);
    struct sockaddr_in peeraddr = getPeerAddr(sockfd);
    return localaddr.sin_port == peeraddr.sin_port
        && localaddr.sin_addr.s_addr == peeraddr.sin_addr.s_addr;
}

int getSocketError(ZL_SOCKET sockfd)
{
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof(optval));

    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        return errno;
    }
    else
    {
        return optval;
    }
}

NAMESPACE_ZL_NET_END
