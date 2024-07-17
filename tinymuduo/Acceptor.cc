#include "Acceptor.h"
#include <sys/types.h>        
#include <sys/socket.h>
#include "EventLoop.h"
#include "Logger.h"
#include "errno.h"

static int createNonBlocking(){
    int fd=::socket(AF_INET,SOCK_NONBLOCK|SOCK_STREAM|SOCK_CLOEXEC,0);
    if(fd==-1){
        LOG_FATAL("%s:%s:%d acceptSocket create fail err:%d",__FILE__,__FUNCTION__,__LINE__,errno);
    }
    return fd;
}

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr)
    :loop_(loop)
    ,listening_(false)
    ,acceptSocket_(createNonBlocking())
    ,acceptChannel_(loop,acceptSocket_.fd()){
        acceptSocket_.bindAddress(listenAddr);
        acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead,this));
}
Acceptor::~Acceptor(){
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceptor::listen(){
    listening_=true;
    /*
        监听了意味着可以接收连接了  加入到epoll监听可读事件
    */
    acceptSocket_.listen();
    acceptChannel_.enableReading();
 
    LOG_INFO("%s:%s:%d begin listening",__FILE__,__FUNCTION__,__LINE__);
}


void Acceptor::handleRead(){
    InetAddress addr;
    int connectfd=acceptSocket_.accept(&addr);
    if(connectfd>=0){
        if(newConnectionCallback_){
            newConnectionCallback_(connectfd,addr); 
        }else{
            close(connectfd);
            LOG_FATAL("%s:%s:%d there no newConnectionCallback err:%d",__FILE__,__FUNCTION__,__LINE__,errno);
        }
    }   
}