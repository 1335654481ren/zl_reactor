#ifndef ZL_EPOLLCLIENT_H
#define ZL_EPOLLCLIENT_H
//// epollʵ�ֵ�ѹ�����Թ��ߣ���������������ֳ����ӣ���һֱ�໥��������

class EpollClient
{
public:
	EpollClient(const char *serverIP, int serverPort, int connNum);
	~EpollClient();

public:
	bool RunLoop();
	bool Start();
	void Stop() { running_ = false; }

private:
	void CreateSockets();
	void SetNonBlock(int sock);
	void AddSock(int sock);
	void DelSock(int sock);
	bool SockWrite(int sockfd, const char *buffer, int len);
	bool SockRead(int sockfd, char *buffer, int len);

private:
	char srvIp_[64];
	short srvPort_;
	int epollFd_;
	int maxConnNum_;
	bool running_;
};
#endif  /* ZL_EPOLLCLIENT_H */
