#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

/// �򵥵Ĵ��������(��ת������)
/// ͨ��per_fork_per_conn��ʽΪÿһ���ͻ������ӿ���һ��������������server����ͨ��
/// ��������������ı��κ���������Ҳ���Ǵ�˵�е�͸��

void parse_args(int argc, char **argv);
void start_accept();
void proxy(int clientfd);
void print_exit(const char* msg)
{
    fprintf(stderr, "error: %s\n", msg);
    exit(1);
}

int kListenPort;                            //���ؼ����˿�
struct sockaddr_in kRemoteServerAddr;       //������������ĵ�ַ

int main(int argc, char **argv)
{
    parse_args(argc, argv);

    start_accept();

    return 0;
}

void parse_args(int argc, char **argv)
{
    if (argc < 4)
    {
        fprintf(stderr, "usage: %s <proxy-port> <host ip> <service-name|port-number> \r\n", argv[0]);
        exit(1);
    }
    
    struct
    {
        char listenPort[16];
        char isolate_host[64];
        char service_name[32];
    }pargs;

    strcpy(pargs.listenPort, argv[1]);
    strcpy(pargs.isolate_host, argv[2]);
    strcpy(pargs.service_name, argv[3]);
    for (size_t i = 0; i < strlen(pargs.listenPort); i++)
    {
        if (!isdigit(*(pargs.listenPort + i)))
        {
            printf("invalid proxy port : %s\r\n", pargs.listenPort);
            exit(0);
        }
    }
    kListenPort = atoi(pargs.listenPort);

    // ���ñ������������ip�Ͷ˿�
    bzero(&kRemoteServerAddr, sizeof(kRemoteServerAddr));
    kRemoteServerAddr.sin_family = AF_INET;

    struct hostent *hostp;
    unsigned long inaddr = ::inet_addr(pargs.isolate_host);
    if (inaddr != INADDR_NONE)
    {
        bcopy(&inaddr, &kRemoteServerAddr.sin_addr, sizeof(inaddr));
    }
    else if ((hostp = ::gethostbyname(pargs.isolate_host)) != NULL)
    {
        bcopy(hostp->h_addr, &kRemoteServerAddr.sin_addr, hostp->h_length);
    }
    else
    {
        printf("%s unknow host \r\n", pargs.isolate_host);
        exit(1);
    }

#define TCP_PROTO "tcp"

    struct servent *servp;
    if ((servp = ::getservbyname(pargs.service_name, TCP_PROTO)) != NULL)
    {
        kRemoteServerAddr.sin_port = servp->s_port;
    }
    else if (::atoi(pargs.service_name) > 0)
    {
        kRemoteServerAddr.sin_port = htons(atoi(pargs.service_name));
    }
    else
    {
        printf("%s invalid unknow service or port\n", pargs.service_name);
        exit(1);
    }

    printf("listen port : %d, remote server : (%s:%s)\n\n", kListenPort, pargs.isolate_host, pargs.service_name);
}

int set_reuseaddr(int fd)
{
    int optval = 1;
    return ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

void start_accept()
{
    int serverfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (serverfd <= 0)
    {
        printf("failed to crate server socket");
        exit(0);
    }
    set_reuseaddr(serverfd);

    struct sockaddr_in servaddr;
    bzero((char*)&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(kListenPort);
    int ret = ::bind(serverfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if (ret < 0)
    {
        printf("failed to bind server socket to specified");
        exit(1);
    }

    ::listen(serverfd, 1024);
    printf("proxy server start, and listening on %d......\n", kListenPort);

    struct sockaddr_in clientaddr;
    socklen_t clientaddrlen = sizeof(clientaddr);
    while (true)
    {
        int clientfd = ::accept(serverfd, (struct sockaddr*)&clientaddr, &clientaddrlen);
        if (clientfd <= 0)
        {
            if (errno == EINTR)
                continue;
            else
                perror("accept: ");
        }

        printf("get one client : %d\n", clientfd);
        int pid = ::fork();
        if (pid < 0)
        {
            perror("fork: ");
            exit(0);
        }
        else if (pid == 0)    // child 
        {
            ::close(serverfd);
            proxy(clientfd);
            exit(0);
        }
        else                 // parent
        {
            ::close(clientfd);
        }
    }
}

int connect_remote_server(int clientfd)
{
    int remotefd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (remotefd < 0)
    {
        print_exit("failed to create socket to host");
    }

    int connstat = ::connect(remotefd, (struct sockaddr *) &kRemoteServerAddr, sizeof(kRemoteServerAddr));
    switch (connstat)
    {
    case 0:
        break;
    case ETIMEDOUT:
    case ECONNREFUSED:
    case ENETUNREACH:       // ���Ӳ��ϱ����������ʱ�͹ر�client
    {
        const char* errormsg = "cannot connect the server";
        ::write(clientfd, errormsg, strlen(errormsg));
        ::close(clientfd);
        exit(1);
        break;
    }
    default:
        print_exit("failed to connect to host");
        return -1;
    }
    return remotefd;
}

#define JUST_BREAK_IF_FAILURE(ret)     if (ret <= 0)  break
#define MAX_IO_BUF_LEN    2048

void proxy(int clientfd)
{
    assert(clientfd > 0);

    int remotefd = connect_remote_server(clientfd);
    assert(remotefd);

    fd_set rdfdset;
    int iolen = 0;
    char buf[MAX_IO_BUF_LEN];
    while (1)
    {
        FD_ZERO(&rdfdset);
        FD_SET(clientfd, &rdfdset);
        FD_SET(remotefd, &rdfdset);
        int ndfs = ::select(FD_SETSIZE, &rdfdset, NULL, NULL, NULL);
        if (ndfs < 0)
        {
            print_exit("select failed");
        }

        if (FD_ISSET(clientfd, &rdfdset))       // �ͻ��˷��������ݣ� �����client���ݲ�ת���������������
        {
            iolen = ::read(clientfd, buf, sizeof(buf));
            if (iolen <= 0)      break;

            iolen = ::write(remotefd, buf, iolen);
            if (iolen <= 0)      break;
        }
        if (FD_ISSET(remotefd, &rdfdset))      // ��������������������ݣ�����ղ�ת����Զ��client
        {
            iolen = ::read(remotefd, buf, sizeof(buf));
            if (iolen <= 0)      break;

            iolen = ::write(clientfd, buf, iolen);
            if (iolen <= 0)      break;
        }
    }

    ::close(remotefd);
    ::close(clientfd);
}
