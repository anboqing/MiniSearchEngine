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

#define MAXLINE 1024
#define OPEN_MAX 100
#define LISTENQ 20
#define SERV_PORT 5555
#define INFTIM 1000

//线程池任务队列结构体
struct task
{
    int fd;            //需要读写的文件描述符

    struct task *next; //下一个任务
};

//用于保存向客户端发送一次消息所需的相关数据
struct user_data
{
    int fd;

    unsigned int n_size;

    char line[MAXLINE];
};

//线程的任务函数
void *readtask(void *args);

void *writetask(void *args);

//声明epoll_event结构体的变量,ev用于注册事件,数组用于回传要处理的事件
struct epoll_event ev, events[20];

int epfd;

pthread_mutex_t mutex;

pthread_cond_t cond1;

struct task *readhead = NULL, *readtail = NULL, *writehead = NULL;

void setnonblocking(int sock)
{
    int opts;

    opts = fcntl(sock, F_GETFL);

    if (opts < 0)
    {
        perror("fcntl(sock,GETFL)");

        exit(1);
    }

    opts = opts | O_NONBLOCK;

    if (fcntl(sock, F_SETFL, opts) < 0)
    {
        perror("fcntl(sock,SETFL,opts)");

        exit(1);
    }
}

int main()
{
    int i, maxi, listenfd, connfd, sockfd, nfds;

    pthread_t tid1, tid2;

    struct task *new_task = NULL;

    struct user_data *rdata = NULL;

    socklen_t clilen;

    pthread_mutex_init(&mutex, NULL);

    pthread_cond_init(&cond1, NULL);

    //初始化用于读线程池的线程，开启两个线程来完成任务，两个线程会互斥地访问任务链表
    pthread_create(&tid1, NULL, readtask, NULL);

    pthread_create(&tid2, NULL, readtask, NULL);

    //生成用于处理accept的epoll专用的文件描述符
    epfd = epoll_create(256);

    struct sockaddr_in clientaddr;

    struct sockaddr_in serveraddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    //把socket设置为非阻塞方式
    setnonblocking(listenfd);

    //设置与要处理的事件相关的文件描述符
    ev.data.fd = listenfd;

    //设置要处理的事件类型，当描述符可读时触发，触发方式为ET模式
    ev.events = EPOLLIN | EPOLLET;

    //注册epoll事件
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);

    bzero(&serveraddr, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;

    const char *local_addr = "127.0.0.1";

    inet_aton(local_addr, &(serveraddr.sin_addr)); //htons(SERV_PORT);

    serveraddr.sin_port = htons(SERV_PORT);

    bind(listenfd, (sockaddr *)&serveraddr, sizeof(serveraddr));

    //开始监听
    listen(listenfd, LISTENQ);

    maxi = 0;

    for ( ; ; )
    {

        //等待epoll事件的发生
        //每个fd并没有同一个特定的events关联，不知道每次会不会改变。 //比如epfd第一次对应events[0]，第二次执行epoll_wait时，却对应events[1]
        //events数组的大小只要能大于最大的活动链接数便可，比如有一万个客户端
        //连接到服务器上，可能同时活跃的只有100个，这样events数组大小为100就可以
        //了，而select维持的描述符集大小就必须为一万，这不仅仅带来空间上的开销。
        //epoll中的events数组中每个描述符都被触发了，而select的描述符集虽然大多数描述符都
        //都没有触发，但仍然需要使用FD_ISSET判断一下；另外，每次将这么大的描述符集
        //从用户空间传到内核空间也是一个很大的开销。
        nfds = epoll_wait(epfd, events, 20, 500);

        //处理所发生的所有事件
        for (i = 0; i < nfds; ++i)
        {

            //这里将监听套接字同连接套接字放在了一起，这样导致每次都要轮询才能获得监听套接字，对其使用
            //accept调用便可获得新的连接套接字，如果能将监听动作放到另外一个线程中就好了
            if (events[i].data.fd == listenfd)
            {
                connfd = accept(listenfd, (sockaddr *)&clientaddr, &clilen);

                if (connfd < 0)
                {
                    perror("connfd<0");

                    exit(1);
                }

                setnonblocking(connfd);

                const char *str = inet_ntoa(clientaddr.sin_addr);

                std::cout << "connec_ from >> " << str << std::endl;

                //设置用于读操作的文件描述符
                ev.data.fd = connfd;

                //设置用于注测的读操作事件
                ev.events = EPOLLIN | EPOLLET;

                //注册ev，每次有新的客户端的连接到服务器，都需要为其生成一个事件
                epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
            }
            else if (events[i].events & EPOLLIN)
            {
                printf("reading!\n");

                if ( (sockfd = events[i].data.fd) < 0) continue;

                new_task = new task();

                new_task->fd = sockfd;

                new_task->next = NULL;

                //添加新的读任务
                pthread_mutex_lock(&mutex);

                if (readhead == NULL)
                {
                    readhead = new_task;

                    readtail = new_task;
                }
                else
                {
                    readtail->next = new_task;

                    readtail = new_task;
                }

                //唤醒所有等待cond1条件的线程
                pthread_cond_broadcast(&cond1);

                pthread_mutex_unlock(&mutex);
            }
            else if (events[i].events & EPOLLOUT)
            {
                rdata = (struct user_data *)events[i].data.ptr;

                sockfd = rdata->fd;

                write(sockfd, rdata->line, rdata->n_size);

                delete rdata;

                //设置用于读操作的文件描述符
                ev.data.fd = sockfd;

                //设置用于注册的读操作事件
                ev.events = EPOLLIN | EPOLLET;

                //修改sockfd上要处理的事件为EPOLIN
                epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);
            }
        }
    }
}

void *readtask(void *args)
{
    int fd = -1;

    unsigned int n;

    //用于把读出来的数据传递出去
    struct user_data *data = NULL;

    while (1)
    {

        //互斥访问任务队列
        pthread_mutex_lock(&mutex);

        //等待到任务队列不为空
        while (readhead == NULL)
            pthread_cond_wait(&cond1, &mutex); //线程阻塞，释放互斥锁，当等待的条件等到满足时，它会再次获得互斥锁

        fd = readhead->fd;

        //从任务队列取出一个读任务
        struct task *tmp = readhead;

        readhead = readhead->next;

        delete tmp;

        pthread_mutex_unlock(&mutex);

        data = new user_data();

        data->fd = fd;

        if ( (n = read(fd, data->line, MAXLINE)) < 0)
        {
            if (errno == ECONNRESET)
                close(fd);
            else
                std::cout << "readline error" << std::endl;

            if (data != NULL) delete data;
        }
        else if (n == 0)
        {
            //客户端关闭了，其对应的连接套接字可能也被标记为EPOLLIN，然后服务器去读这个套接字
            //结果发现读出来的内容为0，就知道客户端关闭了。
            close(fd);

            printf("Client close connect!\n");

            if (data != NULL) delete data;

        }
        else
        {

            std::cout << "read from client: " << data->line << std::endl;

            data->n_size = n;

            //设置需要传递出去的数据
            ev.data.ptr = data;

            //设置用于注测的写操作事件
            ev.events = EPOLLOUT | EPOLLET;

            //修改sockfd上要处理的事件为EPOLLOUT，这将导致数据被发回客户端（在上面的for循环中）
            epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
        }
    }
}