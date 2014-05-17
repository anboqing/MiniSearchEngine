/*
 * Socket.cpp
 *
 *  Created on: 2014年5月3日
 *      Author: anboqing
 */

#include "Socket.h"
#include <string.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>


using namespace std;
Socket::Socket(const char *type)
{
    string tp(type);
    if (tp == string("udp"))
    {
        socket_file_descriptor_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (socket_file_descriptor_ == -1)
        {
            throw std::runtime_error("server socket create");
        }
    }
    else if (tp == string("tcp"))
    {
        socket_file_descriptor_ = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_file_descriptor_ == -1)
        {
            throw std::runtime_error("server socket create");
        }
    }else{
    	socket_file_descriptor_ = -1;
    }
}

bool Socket::bindToAddess(const Address &addr)
{
    int ret = bind(socket_file_descriptor_, addr.getAddressPoint(), addr.length_);
    if (ret >= 0)
    {
        return true;
    }
    else
    {
        throw std::runtime_error("bind to address");
        return false;
    }
}

Socket::~Socket()
{
    // TODO Auto-generated destructor stub
}

void Socket::setnonblocking()
{
    int opts;

    opts = fcntl(socket_file_descriptor_, F_GETFL);

    if (opts < 0)
    {
        throw runtime_error("fcntl(socket_file_descriptor_,GETFL)");
    }

    opts = opts | O_NONBLOCK;

    if (fcntl(socket_file_descriptor_, F_SETFL, opts) < 0)
    {
        throw runtime_error("fcntl(socket_file_descriptor_,SETFL,opts)");
    }
}

int Socket::_listen(int back_logs){
	int iret=listen(socket_file_descriptor_,back_logs);
	if(iret==-1){
		throw runtime_error("listen");
	}
	return iret;
}