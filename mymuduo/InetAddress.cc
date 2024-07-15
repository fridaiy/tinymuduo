#include "InetAddress.h"

#include <string.h>
#include <iostream>


InetAddress::InetAddress(uint16_t port){
    
}
InetAddress::InetAddress(std::string ip,uint16_t port){
    memset(&addr_,0,sizeof(sockaddr_in));
    addr_.sin_family=AF_INET;
    addr_.sin_port=htons(port);
    addr_.sin_addr.s_addr=inet_addr(ip.c_str());
}
InetAddress::InetAddress(const sockaddr_in& addr){
    memset(&addr_,0,sizeof(sockaddr_in));
    addr_=addr;
}
std::string InetAddress::toIp() const{
    char buf[64];
    inet_ntop(AF_INET,&addr_.sin_addr,buf,sizeof(buf));
    return buf;  
}
std::string InetAddress::toIpPort() const{
    char buf[64];
    inet_ntop(AF_INET,&addr_.sin_addr,buf,sizeof(buf));
    size_t end=strlen(buf);

    sprintf(buf+end,":%u",ntohs(addr_.sin_port));
    return buf;
}
uint16_t InetAddress::port() const{
    return ntohs(addr_.sin_port); 
}
const sockaddr_in* InetAddress::getSockAddr()const{
    return &addr_;
}

void InetAddress::setAddr(sockaddr_in addr){
    addr_=addr;
}
/*
int main(){
    InetAddress addr("128.0.2.9",8000);
    std::cout<<addr.toIp()<<std::endl;
    std::cout<<addr.toIpPort()<<std::endl;
    std::cout<<addr.port()<<std::endl;
}*/