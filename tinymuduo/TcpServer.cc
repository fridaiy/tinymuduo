#include "TcpServer.h"
#include "Logger.h"
#include "Acceptor.h"
#include "EventLoopThreadPool.h"
#include "TcpConnection.h"
#include "EventLoop.h"
#include "TcpConnection.h"

#include <functional>
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>
static EventLoop* checkNotNull(EventLoop* loop){
    if(loop==nullptr){
        LOG_FATAL("%s%s%d loop is nullptr",__FILE__,__FUNCTION__,__LINE__);
    }
    return loop;
}

static void printBanner(){
     std::ifstream ifs("banner.txt");

    if (!ifs.is_open()) {
        std::cout<<" _____              ___  ___          _             "<<std::endl;
        std::cout<<"|_   _|_            |  \\/  |         | |            "<<std::endl;
        std::cout<<"  | | (_) _ __ _   _| .  . |_   _  __| |_   _  ___  "<<std::endl;
        std::cout<<"  | | | | '_ \\| | | | |\\/| | | | |/ _` | | | |/ _ \\ "<<std::endl;
        std::cout<<"  | | | | | | | |_| | |  | | |_| | (_| | |_| | (_) |"<<std::endl;
        std::cout<<"  \\_/ |_|_| |_|\\__, \\_|  |_/\\__,_|\\__,_|\\__,_|\\___/ "<<std::endl;
        std::cout<<"                __/ |                               "<<std::endl;
        std::cout<<"               |___/                                "<<std::endl;
    }else{
        std::string line;
        while (std::getline(ifs, line)) {
            std::cout << line << std::endl;
        }
        ifs.close();
    }
}


TcpServer::TcpServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const std::string& nameArg,
            int numThreads)
            :loop_(checkNotNull(loop)),
            name_(nameArg),
            ipPort_(listenAddr.toIpPort()),
            acceptor_(new Acceptor(loop,listenAddr)),
            threadPool_(new EventLoopThreadPool(loop,nameArg,numThreads)),
            nextConnId_(1),
            localAddr_(listenAddr),
            started_(false){
                acceptor_->setNewConnectionCallback(std::bind(&TcpServer::acceptNewConnection,this,
                                                                std::placeholders::_1,
                                                                std::placeholders::_2));
            }

TcpServer::~TcpServer(){
    for (auto &item : connections_){
        /*
            TcpServer对象析构时 如果还存在未释放的连接 释放连接
        */
        TcpConnectionPtr conn(item.second); 
        item.second.reset();
        conn->getLoop()->runInLoop(
            std::bind(&TcpConnection::connectDestroyed, conn)
        );
    }
}

void TcpServer::setThreadNum(int numThreads){
    if(!started_){
        threadPool_->setThreadNum(numThreads);
    }else{
        LOG_ERROR("%s%s%d TcpServer has already start!",__FILE__,__FUNCTION__,__LINE__);
    }
}

void TcpServer::start(){
    if(!started_){
        printBanner();
        started_=true;
      
        acceptor_->listen();
        threadPool_->start(threadInitCallback_);
        //这里不能顺便把loop也开启了？
    }
}
void TcpServer::acceptNewConnection(int sockfd, const InetAddress& peerAddr){
    EventLoop* ioLoop = threadPool_->getNextLoop();
    std::string connName = name_+"-"+ipPort_+"#"+std::to_string(nextConnId_++);

    TcpConnectionPtr conn(new TcpConnection(ioLoop,connName,sockfd,localAddr_,peerAddr));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    //让子loop 所在线程 向epoll中删除fd
    //删除一个连接 分两步 删除TcpServer中的TcpConnection的指针 在epoll中移除该事件
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1)); 
    //让子loop 所在线程 向epoll中注册fd并监听事件
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn){
    //这里这么写 就是期待removeConnectionInLoop 让主loop进行调用
    //调用runInLoop的是谁 就在那个loop的线程中执行
    loop_->runInLoop(
        std::bind(&TcpServer::removeConnectionInLoop, this,conn)
    );
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn){
    connections_.erase(conn->getName());
    EventLoop *ioLoop = conn->getLoop();
    //调用runInLoop的是谁 就在那个loop的线程中执行
    //这里肯定就是TcpConnection指向的某个subLoop
    ioLoop->runInLoop(
        std::bind(&TcpConnection::connectDestroyed,conn)
    );
}