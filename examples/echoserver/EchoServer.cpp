#include <iostream>
#include "net/TcpServer.h"
#include "net/TcpConnection.h"
#include "net/EventLoop.h"
#include "base/ZLog.h"
using namespace std;
using namespace zl::net;
using namespace zl::base;

class EchoServer
{
 public:
  EchoServer(EventLoop* loop, const InetAddress& listenAddr);

  void start();

 private:
  void onConnection(const TcpConnectionPtr& conn);

  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time);

  TcpServer server_;
};

EchoServer::EchoServer(EventLoop* loop, const InetAddress& listenAddr)
  : server_(loop, listenAddr, "EchoServer")
{
  server_.setConnectionCallback(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
  server_.setMessageCallback(std::bind(&EchoServer::onMessage, this, 
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void EchoServer::start()
{
	server_.start();
}

void EchoServer::onConnection(const TcpConnectionPtr& conn)
{
     cout  << conn->localAddress().ipPort() << " is "
           << (conn->connected() ? "UP" : "DOWN"); 
}

void EchoServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
{
  string msg(buf->data());
  cout << conn << " EchoServer::onMessage echo " << msg.size() << " bytes, "
           << "data received at " << time.toString() << "\n";
  //conn->send(msg);
}

int main()
{
  zl_log_instance_create("log", "echoserver");
  EventLoop loop;
  InetAddress listenAddr("127.0.0.1", 8888);
  EchoServer server(&loop, listenAddr);
  server.start();
  loop.loop();
  zl_log_instance_destroy();
}
