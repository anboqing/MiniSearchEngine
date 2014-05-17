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

#include "json/json.h"
#include "Socket.h"
#include "Configure.h"


using namespace std;

int receive(int fd, string &res);

void *thread_func(void *arg);

class DataSet
{
public:
    DataSet() {}
    ~DataSet()
    {
    }

    int sockfd_;

    int maxevents_;

    int seconds_;

    struct epoll_event *p_events_;

};

int main(int argc, char const *argv[])
{

    Configure *pconf = Configure::getInstance();

    int max_events = atoi(pconf->getConfigByName("maxevents").c_str());
    int epoll_wait_seconds = atoi(pconf->getConfigByName("epollwaitseconds").c_str());
    int epoll_size = atoi(pconf->getConfigByName("epollsize").c_str());
    int port = atoi(pconf->getConfigByName("port").c_str());

    const int MAX_EVENTS = max_events;

    struct epoll_event ev, events[MAX_EVENTS];

    int  epollfd;

    /* Set up listening socket--"listen_sock" socket(),bind(),listen()*/

    Address server_addr(htonl(INADDR_ANY), htons(port), AF_INET);

    Address client_addr;

    Socket conn_sock("NULL");

    Socket listen_sock("tcp");

    Socket epoll_sock("NULL");
    // bind
    listen_sock.bindToAddess(server_addr);
    // listen
    listen_sock._listen();

    epollfd = epoll_create(epoll_size);

    if (epollfd == -1)
    {
        throw runtime_error("epoll_create");
    }

    epoll_sock.setSockFd(epollfd);


    DataSet data_set;
    data_set.sockfd_ = epoll_sock.getSockFd();
    data_set.maxevents_ = MAX_EVENTS;
    data_set.p_events_ = events;
    data_set.seconds_ = epoll_wait_seconds;

    pthread_t tid;

    pthread_create(&tid, NULL, thread_func, &data_set);

    while (1)
    {
        int newFd = accept(listen_sock.getSockFd(),
                           client_addr.getAddressPoint(), &(client_addr.length_));
        if (newFd == -1)
        {
            throw runtime_error("accept");
        }

        conn_sock.setSockFd(newFd);

        char *str = inet_ntoa(client_addr.getAddressIn().sin_addr);

        cout << "accapt a connection from " <<  str << endl;

        conn_sock.setnonblocking();

        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = conn_sock.getSockFd();
        // add newfd into events queue
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock.getSockFd(), &ev) == -1)
        {
            throw runtime_error("epoll_ctl : conn_sock");
        }
    }

    return 0;
}

int receive(int fd, string &res)
{
    char buf[128];
    memset(buf, 0, 128);
    int len = recv(fd, buf, sizeof(buf), 0);
    cout << "server recv : " << buf << endl;
    string str(buf);
    res = str;
    return len;
}

void *thread_func(void *arg)
{
    string read_msg;

    DataSet data_set = *((DataSet *)arg);

    int epoll_sock = data_set.sockfd_;
    struct epoll_event *events = data_set.p_events_;
    const int MAX_EVENTS  = data_set.maxevents_;
    int epoll_wait_seconds = data_set.seconds_;

    struct epoll_event ev;

    while (true)
    {
        int nfds = epoll_wait(epoll_sock, events, MAX_EVENTS, epoll_wait_seconds);
        if (nfds == -1)
        {
            throw runtime_error("epoll_pwait");
        }
        for (int ix = 0; ix < nfds; ++ix)
        {
            if (events[ix].events & EPOLLIN)
            {
                //如果是已经连接的用户，并且收到数据，那么进行读入
                int sockfd = events[ix].data.fd;
                int readlen  = receive(sockfd, read_msg);
                if (readlen == 0)
                {
                    close(sockfd);
                    events[ix].data.fd = -1;
                }
                else
                {
                    ev.data.fd = sockfd;
                    //读完后准备写
                    ev.events = EPOLLOUT | EPOLLET;
                    epoll_ctl(epoll_sock, EPOLL_CTL_MOD, sockfd, &ev);
                }
            }
            else if (events[ix].events & EPOLLOUT) // 如果有数据发送
            {
                string msg = read_msg;
                Json::Value root ;
                Json::Value arr ;
                Json::Value elem;
                elem["title"] = msg;
                elem["content"] = msg;
                arr.append(elem);
                root["files"] = arr;
                Json::FastWriter writer ;
                Json::StyledWriter stlwriter ;

                string result_msg = stlwriter.write(root);

                int sockfd = events[ix].data.fd;

                int sendnum = send(sockfd, result_msg.c_str() , result_msg.size(), 0);

                if (sendnum > 0)
                {
                    ev.data.fd = sockfd;
                    //写完后，这个sockfd准备读
                    ev.events = EPOLLIN | EPOLLET;
                    epoll_ctl(epoll_sock, EPOLL_CTL_MOD, sockfd, &ev);
                }
            }
        }
    }

}