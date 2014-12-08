#ifndef ZL_EPOLLCLIENT_H
#define ZL_EPOLLCLIENT_H
//// epollʵ�ֵ�ѹ�����Թ��ߣ���������������ֳ����ӣ���һֱ�໥��������

class EpollClient
{
public:
	EpollClient(const char *serverIP, int serverPort, int connNum);
	~EpollClient();

public:
	bool runLoop();
	bool start();
	void stop() { running_ = false; }

private:
	void createSockets();
	void setNonBlock(int sock);
	void addSock(int sock);
	void delSock(int sock);
	bool sockWrite(int sockfd, const char *buffer, int len);
	bool sockRead(int sockfd, char *buffer, int len);

private:
	char srvIP_[64];
	short srvPort_;
	int epollFd_;
	int maxConnNum_;
	bool running_;
};
#endif  /* ZL_EPOLLCLIENT_H */
