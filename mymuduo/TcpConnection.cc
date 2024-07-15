#include "TcpConnection.h"
#include "Socket.h"
#include "Channel.h"
#include "Logger.h"
#include "EventLoop.h"

#include<functional>

static EventLoop* CheckLoopNotNull(EventLoop *loop){
    if(loop==nullptr){
        LOG_FATAL("%s%s%d TcpConnection loop is null\n",__FILE__,__FUNCTION__,__LINE__);
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop* loop,
                const std::string& name,
                int sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr)
                :loop_(CheckLoopNotNull(loop))
                ,name_(name)
                ,state_(kConnecting)
                ,reading_(true)
                ,localAddr_(localAddr)
                ,peerAddr_(peerAddr)
                ,socket_(new Socket(sockfd))
                ,channel_(new Channel(loop,sockfd))
                ,highWaterMark_(64*1024*1024){
                    
                    channel_->setReadCallback(std::bind(&TcpConnection::handleRead,this,std::placeholders::_1));
                    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite,this));
                    channel_->setErrorCallback(std::bind(&TcpConnection::handleError,this));
                    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose,this));
                    socket_->setKeepAlive(true);

                }


TcpConnection::~TcpConnection(){
   
}
//在需要关注可读事件
void TcpConnection::startRead(){
    loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}

void TcpConnection::startReadInLoop(){
    if (!reading_ || !channel_->isReading()){
        channel_->enableReading();
        reading_ = true;
    }
}
//后续可能不在需要关注可读事件
void TcpConnection::stopRead(){
    loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
}

void TcpConnection::stopReadInLoop(){
    if (reading_ || channel_->isReading()){
        channel_->disableReading();
        reading_ = false;
    }
}



void TcpConnection::connectEstablished(){
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed(){
    if (state_ == kConnected){
        setState(kDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}
void TcpConnection::handleClose(){
    setState(kDisconnected);
    channel_->disableAll();
    
    TcpConnectionPtr guardThis(shared_from_this());
    connectionCallback_(guardThis);
    closeCallback_(guardThis);
}
//将fd中数据读入缓冲区 等待messageCallback处理
void TcpConnection::handleRead(TimeStamp receiveTime){
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if (n > 0){
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if (n == 0){
        handleClose();
    }else{
        errno = savedErrno;
        handleError();
    }
}
//发数据时有数据没发完 放缓冲区里注册读事件 发送
void TcpConnection::handleWrite(){
    if(channel_->isWriting()){
        int savedErrno=0;
        ssize_t n=outputBuffer_.writeFd(channel_->fd(),&savedErrno);
        if(n>0){
            outputBuffer_.retrieve(n);
            if(outputBuffer_.readableBytes()==0){
                channel_->disableWriting();
                if(writeCompleteCallback_){
                    loop_->queueInLoop(std::bind(writeCompleteCallback_,shared_from_this()));
                }
                if(state_==kDisconnecting){
                    shutdownInLoop();
                }
            }

        }

    }
}


void TcpConnection::send(const std::string& buf){
    if(state_==kConnected){
        if(loop_->isInLoopThread()){
            sendInLoop(buf.c_str(),buf.size());
        }else{
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop,this,buf.c_str(),buf.size()));
        }
    }
}

void TcpConnection::sendInLoop(const void* message, size_t len){
    ssize_t nwrote=0;
    size_t remaining=len;
    bool faultError =false;
    if(state_==kDisconnecting){
        return ;
    }
    //第一次写数据且缓冲区中没有待发送数据
    if(!channel_->isWriting()&&outputBuffer_.readableBytes()==0){
        nwrote=::write(channel_->fd(),message,len);
        if(nwrote>=0){
            remaining=len-nwrote;
            if(remaining==0&&writeCompleteCallback_){
                loop_->queueInLoop(std::bind(&TcpConnection::writeCompleteCallback_,shared_from_this()));
            }
        }else{
            //出错
            nwrote=0;
            if(errno!=EWOULDBLOCK){//非阻塞没数据正常返回
                //真正错误
                if(errno==EPIPE||errno==ECONNRESET){//接收到对端的重置了
                    faultError=true;
                }
            }
        }
    }
    if(!faultError&&remaining>0){//这次write没发送出去剩余数据存入缓冲区然后给channel注册epollout
    //poller发现tcp返送缓冲有空间就会通知socket 调用handwrite
        ssize_t oldLen=outputBuffer_.readableBytes();
        if(oldLen+remaining>=highWaterMark_&&oldLen<highWaterMark_&&highWaterMarkCallback_){
            loop_->queueInLoop(std::bind(highWaterMarkCallback_,shared_from_this(),oldLen+remaining));
        }
        outputBuffer_.append((char*)message+nwrote,remaining);
        if(!channel_->isWriting()){
            channel_->enableReading();
        }
    }

}


void TcpConnection::shutdown(){
    if(state_==kConnected){ 
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop,this));
    }
}

void TcpConnection::shutdownInLoop(){
    if(!channel_->isWriting()){
        socket_->shutdownWrite();
    }
}

void TcpConnection::handleError(){
    int optval;
    socklen_t optlen = sizeof optval;
    int err = 0;
    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        err = errno;
    }
    else
    {
        err = optval;
    }
    LOG_ERROR("TcpConnection::handleError name:%s - SO_ERROR:%d \n", name_.c_str(), err);
}