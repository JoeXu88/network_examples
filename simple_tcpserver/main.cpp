#include <string>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>


int main()
{
	int sock_fd, conn_fd;
	struct sockaddr_in server_addr, client_addr;
	int port_number = 9099;

	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr,"Socket error:%s\n\a", strerror(errno));
		exit(1);
	}

	bzero(&server_addr, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port_number);

	if (bind(sock_fd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr)) == -1) {
		fprintf(stderr,"Bind error:%s\n\a", strerror(errno));
		exit(1);
	}

	if(listen(sock_fd, 1024) == -1) {
		fprintf(stderr,"Listen error:%s\n\a", strerror(errno));
		exit(1);
	}

	printf("waiting for clients....\n");
	socklen_t addr_len = sizeof(struct sockaddr);
	conn_fd = accept(sock_fd, (struct sockaddr *)(&client_addr), &addr_len);
	if(conn_fd == -1)
	{
		fprintf(stderr,"accpet error:%s\n\a", strerror(errno));
		exit(1);
	}

	char addr[16];
	std::string client_addr_ip = inet_ntop(AF_INET, (void*)&client_addr, (char*)&addr[0], sizeof(addr));

	printf("new client conneted: %s\n", client_addr_ip.c_str());

	int ret = -1;
	char buff[1024] = {0};
	while(true)
	{
		memset(buff, 0, sizeof(buff));
		ret = recv(conn_fd, buff, sizeof(buff), 0);
		if(ret <= 0)
		{
			printf("client closed\n");
			conn_fd = accept(sock_fd, (struct sockaddr *)(&client_addr), &addr_len);
			if(conn_fd == -1)
			{
				fprintf(stderr,"accpet error:%s\n\a", strerror(errno));
				exit(1);
			}
			printf("new client conneted: %s\n", client_addr_ip.c_str());
			// break;
		}
		else
		{
			printf("got msg from client: %s, len: %d\n", buff, ret);
			std::string msg = "{\"serial\": \"\", \"cmd\": \"stream_keepalive_mgr_req\"}";
			send(conn_fd, (const uint8_t*)msg.c_str(), msg.length(), 0);
		}
		usleep(10000);
	}
	close(conn_fd);
	close(sock_fd);
	printf("server exit!\n");

	return 0;
}
