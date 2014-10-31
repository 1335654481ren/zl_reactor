#include "net/Acceptor.h"
#include "net/Socket.h"
#include "net/Channel.h"
#include "net/EventLoop.h"
#include "base/ZLog.h"
#include "net/ActiveSocket.h"
using namespace zl::base;
NAMESPACE_ZL_NET_START

Acceptor::Acceptor(EventLoop *loop, const InetAddress& listenAddr) 
    : loop_(loop)
{
	accept_socket = new Socket();
	if (!accept_socket->create())
	{
		throw SocketException("Could not create server socket.");
	}
	accept_socket->setNoDelay();

	if (!accept_socket->setReuseAddr(true))
	{
		throw SocketException("Could not reuse socket address.");
	}
	if (!accept_socket->bind(listenAddr))
	{
		throw SocketException("Could not bind to port.");
	}

	accept_channel_ = new Channel(loop, accept_socket->getSocket());
	accept_channel_->setReadCallback(std::bind(&Acceptor::onAccept, this));
}

Acceptor::~Acceptor()
{
	accept_socket->close();
	Safe_Delete(accept_channel_);
	Safe_Delete(accept_socket);
}

void Acceptor::listen()
{
	if (!accept_socket->listen(64))
	{
		throw SocketException("Could not listen to port.");
	}
	LOG_INFO("server host [%s]", accept_socket->getHost().c_str());

	accept_channel_->enableReading();
}

void Acceptor::onAccept()
{
	while (true)
	{
		ActiveSocket *socket = new ActiveSocket;
		if (accept_socket->accept(*socket))
		{
			if (newConnCallBack_)
			{
				newConnCallBack_(socket);
			}
			else
			{
				LOG_ALERT("no callback on Acceptor::OnAccept(), and close the coming connection!");
				socket->close();
				Safe_Delete(socket);
			}
			break;
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