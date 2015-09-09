#ifndef ZL_EchoClient_H
#define ZL_EchoClient_H
//// epollʵ�ֵ�ѹ�����Թ��ߣ���������������ֳ����ӣ���һֱ�໥��������

class EpollPoller;

class EchoClient
{
public:
    EchoClient(const char *serverIP, int serverPort, int connNum);
    ~EchoClient();

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
    int maxConnNum_;
    bool running_;
    EpollPoller* epoller_;
};
#endif  /* ZL_EchoClient_H */
