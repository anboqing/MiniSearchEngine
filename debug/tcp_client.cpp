#include "nethead.h"
#include <stdexcept>
#include <string.h>
#include <iostream>

using namespace std;

int main(int argc, char const *argv[])
{
    int clientsock;
    struct sockaddr_in server_addr;

    clientsock = socket(AF_INET, SOCK_STREAM, 0);

    if (clientsock == -1)
    {
        throw runtime_error("client socket()");
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8888);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    socklen_t len = sizeof(server_addr);

    int iret = connect(clientsock, (struct sockaddr *)&server_addr, len);
    if (iret == -1)
    {
        throw runtime_error("connect");
    }

    while (1)
    {
        char buf[512] = {0};
        send(clientsock, "hello", 6, 0);     //向clientsock服务端发送数据
        recv(clientsock, buf, sizeof(buf), 0); //从clientsock服务端接收数据
        cout << buf << endl;
        memset(buf,0,512);
        sleep(1);
    }
    
    return 0;
}