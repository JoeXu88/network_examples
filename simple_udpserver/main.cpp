#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <stdio.h>

static const int DEFAULT_PORT = 9000;
static const int MAX_SIZE = 4*1024;

int main(int argc, char** argv)
{
    int listen_port = DEFAULT_PORT;
    if(argc > 1)
    {
        listen_port = std::stoi(argv[1]);
    }
    printf("listen on port: %d\n", listen_port);

    struct sockaddr_in sock_addr;
    bzero(&sock_addr, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_addr.sin_port = listen_port;

    int sockfd_listen = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd_listen == -1)
    {
        printf("can not create udp listen socket\n");
        return -1;
    }

    if(bind(sockfd_listen, (struct sockaddr *)(&sock_addr), sizeof(struct sockaddr)) == -1)
    {
        printf("can not bind server sock\n");
        return -1;
    }

    char recvbuf[MAX_SIZE] = {0};
    struct sockaddr_in sock_addr_client;
    socklen_t addr_len = sizeof(sock_addr_client);
    while(true)
    {
        int n = recvfrom(sockfd_listen, recvbuf, MAX_SIZE, 0, (struct sockaddr *)(&sock_addr_client), &addr_len);
        recvbuf[n] = 0;
        printf("got mesg: %s\n", recvbuf);
    }

    close(sockfd_listen);
    return 0;
}