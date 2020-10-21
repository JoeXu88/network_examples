#include <string>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>

const uint32_t MAX_SIZE = 10240;

int main()
{
    std::string server_host = "127.0.0.1";
    int sock_fd;
	struct sockaddr_in server_addr;
	int port_number = 9099;

	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr,"client Socket error:%s\n\a", strerror(errno));
        return -1;
	}

    struct hostent *ht = NULL;
    ht = gethostbyname(server_host.c_str());
    if(ht == NULL || ht->h_addr_list[0] == NULL)
    {
        printf("can not get host from %s", server_host.c_str());
        return -1;
    }

    char* addr = ht->h_addr_list[0];


    bzero(&server_addr, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
    server_addr.sin_addr = *((struct in_addr*)addr);
    // inet_pton(AF_INET, server_host.c_str(), &server_addr.sin_addr);
	server_addr.sin_port = htons(port_number);

    int connect_ret = connect(sock_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if(connect_ret < 0)
    {
        printf("can not connect to server: %s\n", server_host.c_str());
        return -1;
    }

    // char msg[MAX_SIZE] = {0};
    // memset(msg, '1', MAX_SIZE);

    std::string msg = "hello from client";
    int total = msg.length();
    int left = total;
    while(left)
    {
        int n_sent = send(sock_fd, msg.c_str(), msg.length(), 0);
        left = total - n_sent;
    }

    close(sock_fd);

    return 0;
}