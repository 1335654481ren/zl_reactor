#include "net/TcpAcceptor.h"
#include "net/Socket.h"
#include "net/Channel.h"
#include "net/EventLoop.h"
#include "base/ZLog.h"
#include "net/ActiveSocket.h"
#include "net/InetAddress.h"
using namespace zl::base;
NAMESPACE_ZL_NET_START

TcpAcceptor::TcpAcceptor(EventLoop *loop, const InetAddress& listenAddr)
    : loop_(loop)
{
    accept_socket = new Socket(SocketUtil::createSocket());

    accept_socket->setNoDelay();
    accept_socket->setNonBlocking();

    if (!accept_socket->setReuseAddr(true))
    {
        throw SocketException("Could not reuse socket address.");
    }
    if (!accept_socket->bind(listenAddr))
    {
        throw SocketException("Could not bind to port.");
    }

    accept_channel_ = new Channel(loop, accept_socket->fd());
    accept_channel_->setReadCallback(std::bind(&TcpAcceptor::onAccept, this, std::placeholders::_1));
}

TcpAcceptor::~TcpAcceptor()
{
    accept_channel_->disableAll();
    accept_channel_->remove();
    Safe_Delete(accept_channel_);
    Safe_Delete(accept_socket);
}

void TcpAcceptor::listen()
{
    loop_->assertInLoopThread();
    if (!accept_socket->listen(128)) //may be bigger, see 'cat /proc/sys/net/core/somaxconn'
    {
        throw SocketException("Could not listen to port.");
    }
    LOG_INFO("TcpAcceptor::listen on [%s]", accept_socket->getHost().c_str());

    accept_channel_->enableReading();
}

void TcpAcceptor::onAccept(Timestamp now)
{
    loop_->assertInLoopThread();
	int count = 0;
    while(count < 100)
    {
        InetAddress peerAddr;
        ZL_SOCKET newfd = accept_socket->accept(&peerAddr);
        if(newfd > 0)
        {
            if (newConnCallBack_)
            {  
                LOG_INFO("TcpAcceptor::OnAccept accept one client from[%d][%s]", newfd, peerAddr.ipPort().c_str());
                newConnCallBack_(newfd, peerAddr);
            }
            else
            {
                LOG_ALERT("TcpAcceptor::OnAccept() no callback , and close the coming connection![%d]", newfd);
            }
			count ++;
        }
        else
        {
            if(errno != EAGAIN)
                LOG_ALERT("TcpAcceptor::OnAccept() accept connection fail![%d][%d]", newfd, errno);
            //  11 : EAGAIN���׽��ִ��ڷ�����״̬����ǰû����������
            //	EBADF���Ƿ����ļ���������
            //	ECONNABORTED�������жϡ�
            //	EINTR��ϵͳ���ñ��ź��жϡ�
            //	EINVAL���׽���û�д��ڼ���״̬����Ƿ���addrlen������
            //	EMFILE���ﵽ���̴��ļ����������ơ�
            //	ENFILE���ﵽ���ļ������ơ�
            //	ENOTSOCK���ļ�������Ϊ�ļ����ļ���������
            //	EOPNOTSUPP���׽������Ͳ���SOCK_STREAM��
            break;
        }
    }
}

NAMESPACE_ZL_NET_END
