#pragma once 

#include "noncopyable.h"
#include "Channel.h"
#include "Socket.h"
#include "InetAddress.h"
#include <functional>

class EventLoop;

class Acceptor:noncopyable{
    /*
        这东西acceptor 其实就是对listenfd的封装么  
        题外话为有channel了还要加一个socket属性呢？ 其实这个socket主要的作用
        不是存储socketfd 而是调用网络编程(socket)的那些系统调用 什么bind listen accept

        这里有两个函数 一个是handlread这个是处理 listen上的可读事件
        比如把新连接找到一个loop注册到对应的poller中
        或者错误事件
        像是newConnectioncb这种回调函数 都是要从外面传入的
    */
public:
    using NewConnectionCallback=std::function<void(int sockfd,const InetAddress&)>;
    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
    ~Acceptor();
    void setNewConnectionCallback(const NewConnectionCallback& cb){ newConnectionCallback_ = std::move(cb); }
    void listen();
    bool listening() const { return listening_; }
private:
    bool listening_;
    /*
        handleRead仅仅是accept返回一个fd 和对端的地址
        也没选择子loop 也没注册 poller
        而是选择调用回调 看来回调才是重中之重啊   
    */
    void handleRead();
    Channel acceptChannel_;
    Socket acceptSocket_;
    EventLoop* loop_;
    NewConnectionCallback newConnectionCallback_;
};







