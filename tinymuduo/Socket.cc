#include "Socket.h"
#include "Logger.h"

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/tcp.h>

Socket::~Socket(){
    close(sockfd_);
}


void  Socket::bindAddress(const InetAddress& localaddr){
    int res=::bind(sockfd_,(sockaddr *)localaddr.getSockAddr(),sizeof(sockaddr_in));
    if(res!=0){
        LOG_FATAL("bind fail");
    }
} 

void  Socket::listen(){
    if(0!=::listen(sockfd_,1024)){
        LOG_FATAL("listen fail");
    }
}

int  Socket::accept(InetAddress* peeraddr){
    sockaddr_in addr;
    memset(&addr,0,sizeof addr);
    socklen_t len;
    int fd=::accept(sockfd_,(sockaddr*)&addr,&len);
    if(fd>=0){
        peeraddr->setAddr(addr);
    }
    return fd;
}   

void  Socket::shutdownWrite(){
    //关闭写端
    if(::shutdown(sockfd_,SHUT_WR)<0){
        LOG_ERROR("shutdown");
    }
}

void  Socket::setTcpNoDelay(bool on){
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
               &optval, static_cast<socklen_t>(sizeof optval));
}
void  Socket::setReuseAddr(bool on){
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
               &optval, static_cast<socklen_t>(sizeof optval));
}
void  Socket::setReusePort(bool on){
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
                         &optval, static_cast<socklen_t>(sizeof optval));
}
void  Socket::setKeepAlive(bool on){
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,&optval, static_cast<socklen_t>(sizeof optval));
}