#pragma once 

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <string>
class InetAddress{
private:
    sockaddr_in addr_;
public:
    explicit InetAddress(uint16_t port = 0);
    explicit InetAddress(std::string ip,uint16_t port = 0);
    explicit InetAddress(const sockaddr_in& addr);
    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t port() const;
    const sockaddr_in* getSockAddr() const;
    void setAddr(sockaddr_in addr);
};