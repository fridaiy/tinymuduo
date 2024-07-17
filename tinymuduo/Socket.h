#pragma once

#include "noncopyable.h"
#include "InetAddress.h"

class Socket : noncopyable{
    /*
        感觉就是对socket fd和常用方法封装在一起了
    */
public:
    explicit Socket(int sockfd)
        : sockfd_(sockfd)
    { }
    ~Socket();
    int fd() const { return sockfd_; }
    void bindAddress(const InetAddress& localaddr);
    void listen();
    int accept(InetAddress* peeraddr);
    void shutdownWrite();
    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);
private:
    const int sockfd_;
};