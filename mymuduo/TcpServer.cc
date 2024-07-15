#include "TcpServer.h"
#include "Logger.h"
#include "Acceptor.h"
#include "EventLoopThreadPool.h"
#include "TcpConnection.h"
#include "EventLoop.h"
#include "TcpConnection.h"

#include <functional>
#include <string.h>

static EventLoop* checkNotNull(EventLoop* loop){
    if(loop==nullptr){
        LOG_FATAL("%s%s%d loop is nullptr",__FILE__,__FUNCTION__,__LINE__);
    }
    return loop;
}



TcpServer::TcpServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const std::string& nameArg,
            Option option = kNoReusePort)
            :loop_(checkNotNull(loop)),
            name_(nameArg),
            ipPort_(listenAddr.toIpPort()),
            acceptor_(new Acceptor(loop,listenAddr,kNoReusePort)),
            threadPool_(new EventLoopThreadPool(loop,nameArg)),
            nextConnId_(1),
            localAddr_(listenAddr),
            started_(false){
            
                acceptor_->setNewConnectionCallback(std::bind(&TcpServer::acceptNewConnection,this,
                                                                std::placeholders::_1,
                                                                std::placeholders::_2));

    

            }

TcpServer::~TcpServer(){
    for (auto &item : connections_){
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
    }
}

void TcpServer::start(){
    if(!started_){
        started_=true;
        acceptor_->listen();
    }
}
void TcpServer::acceptNewConnection(int sockfd, const InetAddress& peerAddr){

    EventLoop* ioLoop = threadPool_->getNextLoop();

    std::string connName = name_+"-"+ipPort_+"#"+std::to_string(nextConnId_++);


    TcpConnectionPtr conn(new TcpConnection(ioLoop,
                                            connName,
                                            sockfd,
                                            localAddr_,
                                            peerAddr));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1)); 
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn){
    loop_->runInLoop(
        std::bind(&TcpServer::removeConnectionInLoop, this, conn)
    );
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn){
    connections_.erase(conn->getName());
    EventLoop *ioLoop = conn->getLoop(); 
    ioLoop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn)
    );
}