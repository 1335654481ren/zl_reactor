#include "net/Acceptor.h"
#include "net/Socket.h"
#include "net/Channel.h"
#include "net/EventLoop.h"
#include "base/ZLog.h"
#include "net/ActiveSocket.h"
#include "net/InetAddress.h"
using namespace zl::base;
NAMESPACE_ZL_NET_START

Acceptor::Acceptor(EventLoop *loop, const InetAddress& listenAddr)
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
    accept_channel_->setReadCallback(std::bind(&Acceptor::onAccept, this, std::placeholders::_1));
}

Acceptor::~Acceptor()
{
    Safe_Delete(accept_channel_);
    Safe_Delete(accept_socket);
}

void Acceptor::listen()
{
    if (!accept_socket->listen(64))
    {
        throw SocketException("Could not listen to port.");
    }
    LOG_INFO("Acceptor::listen on [%s]", accept_socket->getHost().c_str());

    accept_channel_->enableReading();
}

void Acceptor::onAccept(Timestamp now)
{
    while (true)
    {
        //ActiveSocket socket;
        //if (accept_socket->accept(socket)) //���������ˣ���ΪActiveSocket����ʱ��ر�socket fd
        InetAddress peerAddr;
        ZL_SOCKET newfd = accept_socket->accept(&peerAddr);
        if(newfd > 0)
        {
            if (newConnCallBack_)
            {  
                LOG_INFO("accept one client on Acceptor::OnAccept [%s]", peerAddr.ipPort().c_str());
                newConnCallBack_(newfd, peerAddr);
            }
            else
            {
                LOG_ALERT("no callback on Acceptor::OnAccept(), and close the coming connection!");
            }
        }
        else
        {
            LOG_ALERT("accept connection fail on Acceptor::OnAccept()!");
            //if (errno == )
            //EAGAIN���׽��ִ��ڷ�����״̬����ǰû����������
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
