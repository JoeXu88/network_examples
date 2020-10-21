#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <stdio.h>
#include <thread>
#include <chrono>

static const int DEFAULT_PORT = 9000;
static const char* serv_addr = "127.0.0.1";
static const int MAX_SIZE = 1024;

int main(int argc, char** argv)
{
    int serv_port = DEFAULT_PORT;
    if(argc > 1)
    {
        serv_port = std::stoi(argv[1]);
    }
    printf("use serv port: %d\n", serv_port);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd == -1)
    {
        printf("can not create sock fd\n");
        return -1;
    }

    struct sockaddr_in sock_addr_srv;
    bzero(&sock_addr_srv, sizeof(sock_addr_srv));
    sock_addr_srv.sin_family = AF_INET;
    sock_addr_srv.sin_port = serv_port;
    inet_pton(AF_INET, serv_addr, &sock_addr_srv.sin_addr);

    socklen_t addr_len = sizeof(sock_addr_srv);

    char send_msg[MAX_SIZE] = {0};
    int cnt = 0;
    while(true)
    {
        sprintf(send_msg, "now send cnt:%d", cnt++);
        int ret = sendto(sockfd, send_msg, MAX_SIZE, 0, (struct sockaddr *)(&sock_addr_srv), addr_len);
        printf("send ret: %d\n", ret);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    close(sockfd);
    return 0;
}