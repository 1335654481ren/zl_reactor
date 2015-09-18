#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "EchoClient.h"

EchoClient *pollclient = NULL;

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("usage : [%s] serverip serverport max_client_sockets; example: [%s] 127.0.0.1 5000 1000 \n", argv[0], argv[0]);
        return 0;
    }

    pollclient = new EchoClient(argv[1], atoi(argv[2]), atoi(argv[3]));
    pollclient->start();

    pollclient->runLoop();

    return 0;
}
