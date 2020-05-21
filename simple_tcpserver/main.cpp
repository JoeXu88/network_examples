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

	int flags =1;
	setsockopt(sock_fd, SOL_TCP, TCP_NODELAY, (void *)&flags, sizeof(flags));
	setsockopt(conn_fd, SOL_TCP, TCP_NODELAY, (void *)&flags, sizeof(flags));
	setsockopt(conn_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&flags, sizeof(flags));
	setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&flags, sizeof(flags));

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

	int opt = 0;
	socklen_t len=sizeof(int);
	if((getsockopt(sock_fd,SOL_TCP,TCP_NODELAY,(char*)&opt,&len))==0){
		printf("listen sock TCP_NODELAY Value: %d\n", opt);
	}

	opt = 0;
	if((getsockopt(conn_fd,SOL_TCP,TCP_NODELAY,(char*)&opt,&len))==0){
		printf("connc sock TCP_NODELAY Value: %d\n", opt);
	}

	opt = 0;
	if((getsockopt(conn_fd,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,&len))==0){
		printf("connc sock SO_REUSEADDR Value: %d\n", opt);
	}

	uint8_t buf[sizeof(struct in6_addr)];
	inet_pton(AF_INET, "127.0.0.1", buf);
	printf("buf: ");
	for(int i=0; i<sizeof(struct in6_addr); i++)
		printf("%x", buf);
	printf("\n");

	char addr[INET_ADDRSTRLEN] = {0};
	std::string client_addr_ip = inet_ntop(AF_INET, (void*)&client_addr.sin_addr, (char*)&addr[0], sizeof(addr));
	printf("new client conneted: %s, %s\n", client_addr_ip.c_str(), inet_ntoa(client_addr.sin_addr));

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
			std::string msg = "{\"serial\": \"1\", \"cmd\": \"stream_keepalive_mgr_req\"}\r\n\r\n";
			printf("send back to client: %s", msg.c_str());
			send(conn_fd, (const uint8_t*)msg.c_str(), msg.length(), 0);
		}
		usleep(10000);
	}
	close(conn_fd);
	close(sock_fd);
	printf("server exit!\n");

	return 0;
}
