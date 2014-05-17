#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <stdexcept>


#include "Socket.h"

using namespace std;

int receive(int fd);

int main(int argc, char const *argv[])
{
    const int MAX_EVENTS = 10;

    struct epoll_event ev, events[MAX_EVENTS];

    int nfds, epollfd;

    /* Set up listening socket--"listen_sock" socket(),bind(),listen()*/

    Address server_addr(htonl(INADDR_ANY), htons(8888), AF_INET);

    Address client_addr;

    Socket conn_sock("NULL");

    Socket listen_sock("tcp");

    Socket epoll_sock("NULL");
    // bind
    listen_sock.bindToAddess(server_addr);
    // listen
    listen_sock._listen();

    epollfd = epoll_create(256);
    if (epollfd == -1)
    {
        throw runtime_error("epoll_create");
    }

    epoll_sock.setSockFd(epollfd);

    ev.events = EPOLLIN;
    ev.data.fd = listen_sock.getSockFd();

    if (epoll_ctl(epoll_sock.getSockFd(), EPOLL_CTL_ADD, listen_sock.getSockFd(), &ev) == -1)
    {
        throw runtime_error("epoll_ctl:listen_sock");
    }

    while (true)
    {
        nfds = epoll_wait(epoll_sock.getSockFd(), events, MAX_EVENTS, 5);
        if (nfds == -1)
        {
            throw runtime_error("epoll_pwait");
        }
        for (int ix = 0; ix < nfds; ++ix)
        {
            if (events[ix].data.fd == listen_sock.getSockFd())//如果新监测到一个SOCKET用户连接到了绑定的SOCKET端口，建立新的连接。
            {
                int ret = accept(listen_sock.getSockFd(),
                                 client_addr.getAddressPoint(), &(client_addr.length_));
                conn_sock.setSockFd(ret);
                if (ret == -1)
                {
                    throw runtime_error("accept");
                }
                char *str = inet_ntoa(client_addr.getAddressIn().sin_addr);
                cout << "accapt a connection from " <<  str << endl;

                conn_sock.setnonblocking();

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = conn_sock.getSockFd();

                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock.getSockFd(), &ev) == -1)
                {
                    throw runtime_error("epoll_ctl:conn_sock");
                }
            }
            else if (events[ix].events & EPOLLIN)
            {
                //如果是已经连接的用户，并且收到数据，那么进行读入
                int sockfd = events[ix].data.fd;
                int readlen = receive(sockfd);

                if (readlen == 0)
                {
                    //
                    close(sockfd);
                    events[ix].data.fd = -1;
                }

                ev.data.fd = sockfd;
                //读完后准备写
                ev.events = EPOLLOUT | EPOLLET;
                epoll_ctl(epoll_sock.getSockFd(), EPOLL_CTL_MOD, sockfd, &ev);

            }
            else if (events[ix].events & EPOLLOUT) // 如果有数据发送

            {
                int sockfd = events[ix].data.fd;

                send(sockfd, "hello clinet", 13, 0);

                ev.data.fd = sockfd;
                //写完后，这个sockfd准备读
                ev.events = EPOLLIN | EPOLLET;
                epoll_ctl(epoll_sock.getSockFd(), EPOLL_CTL_MOD, sockfd, &ev);
            }
        }
    }
    return 0;
}

int receive(int fd)
{
    char buf[128];
    memset(buf, 0, 128);
    int len = recv(fd, buf, sizeof(buf), 0);
    cout << "server recv : " << buf << endl;
    return len;
}