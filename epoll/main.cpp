#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <thread>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <chrono>

const int EPOLL_SOKET_SIZE = 1024; //this is deprecated since Linux 2.6.8, but we must set a value > 0
const int port = 9099;

int set_sock_nblock(int skt, bool ublock)
{
    // int nblock = ublock;
    // int ret = ioctl(skt, FIONBIO, &nblock);
    // if(ret == -1)
    // {
    //     printf("set %d non block failed\n", skt);
    //     return -1;
    // }

    if(!ublock) return 0;

    int flags = fcntl (skt, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl (skt, F_SETFL, flags);

    return 0;
}

int main()
{
    int listen_sd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_sd == -1)
    {
        printf("can not create listen fd\n");
        return -1;
    }

    if(set_sock_nblock(listen_sd, true))
        return -1;

    struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(listen_sd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr)) == -1) {
		fprintf(stderr,"Bind error:%s\n\a", strerror(errno));
        return -1;
	}

    if(listen(listen_sd, 1024) == -1)
    {
        printf("can not listen correct\n");
        return -1;
    }

    auto func = [&]() {
    int epoll_fd = epoll_create(EPOLL_SOKET_SIZE);
    if(epoll_fd == -1)
    {
        printf("can not create epoll fd\n");
        return -1;
    }

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    // event.events = EPOLLIN;
    event.data.fd = listen_sd;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_sd, &event) == -1)
    {
        printf("can not add server listen event\n");
        return -1;
    }

    struct epoll_event events[EPOLL_SOKET_SIZE];
    char buff[1024] = {0};
    int ready_fds;
    memset(&events, 0, sizeof(events));
    while(true)
    {
        //collect events
        //int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
        // printf("\n%d wait for events\n", epoll_fd);
        ready_fds = epoll_wait(epoll_fd, events, EPOLL_SOKET_SIZE, -1);
        if(ready_fds == -1)
        {
            printf("%d epoll wait err\n", epoll_fd);
            break;
        }


        //测试惊群效应，休眠一段时间以保证事件处理延后，然后其余监听的epoll 也能被调到
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        //process events
        for(int i=0; i<ready_fds; i++)
        {
            int fd = events[i].data.fd;
            printf("%d got events %d\n", epoll_fd, events[i].data.fd);
            if(fd == listen_sd)
            {
                struct sockaddr_in client_addr;
                socklen_t addr_len = sizeof(struct sockaddr);
                int client_fd = -1;
                // while(true)
                {
                client_fd = accept(listen_sd, (struct sockaddr *)(&client_addr), &addr_len);
                if(client_fd == -1)
                {
                    printf("%d accept failed, errno: %d\n", epoll_fd, errno);
                    continue;
                }
                }

                if(set_sock_nblock(client_fd, true)) break;
                printf("%d accept %d ok\n", epoll_fd, client_fd);

                struct epoll_event event;
                event.events = EPOLLIN | EPOLLOUT | EPOLLHUP;
                event.data.fd = client_fd;
                if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1)
                {
                    printf("epoll add client fd err");
                    continue;
                }
            }
            else
            {
                // printf("process client %d:\n", fd);
                int ret = recv(fd, buff, sizeof(buff), 0);
                if(ret <= 0)
                {
                    printf("client %d closed\n", fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                    continue;
                }

                buff[ret] = '\0';
                printf("%d read from client %d: %s\n", epoll_fd, fd, buff);

                // send(fd, buff, ret, 0);
            }
        }
    }
    };

    std::thread t1(func);
    std::thread t2(func);

    t1.join();
    t2.join();

    return 0;
}
