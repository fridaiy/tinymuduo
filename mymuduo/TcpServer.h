#pragma once

#include "EventLoop.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "noncopyable.h"
#include "EventLoopThreadPool.h"
#include "Callbacks.h"
#include "TcpConnection.h"
#include "Buffer.h"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <atomic>


class TcpServer{
public:
    using ThreadInitCallback=std::function<void(EventLoop*)>;
    
    enum Option{
        kNoReusePort,
        kReusePort,
    };

    TcpServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const std::string& nameArg,
            Option option);
    ~TcpServer(); 

    void setThreadNum(int numThreads);

    void start();
    
    
    const std::string& ipPort() const { return ipPort_; }
    const std::string& name() const { return name_; }
    EventLoop* getLoop() const { return loop_; }
    std::shared_ptr<EventLoopThreadPool> threadPool(){ return threadPool_; }


    void setConnectionCallback(const ConnectionCallback& cb){ connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb){ messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb){ writeCompleteCallback_ = cb; }
    void setThreadInitCallback(const ThreadInitCallback& cb){ threadInitCallback_ = cb; }
private:
    using ConnectionMap=std::unordered_map<std::string,TcpConnectionPtr>;
    EventLoop* loop_;
    
    const std::string ipPort_;
    const std::string name_;
    const InetAddress localAddr_;
    
    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;
    std::atomic_bool started_;
    
    int nextConnId_;

    ConnectionMap connections_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    ThreadInitCallback threadInitCallback_;


    void acceptNewConnection(int sockfd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

};