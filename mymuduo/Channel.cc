#include "Channel.h"
#include "EventLoop.h"
#include <sys/epoll.h>

const int Channel::kNoneEvent = 0;
//EPOLLPRI用于监视fd上的紧急事件或者高优先级事件如tcp带外数据
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop, int fd)
    :loop_(loop)
    ,fd_(fd)
    ,events_(0)
    ,revents_(0)
    ,index_(-1)//和 epollpoller 中初始化的三个index对应上了
    ,tied_(false){

}

Channel::~Channel(){
    
    //调用evenloop中的方法
}
//什么时候调用？
void Channel::tie(const std::shared_ptr<void>&obj){
    tie_=obj;
    tied_=true;
}
//应该是通过eventloop 调用poller 将fd事件同步到epoll中
void Channel::update(){
    loop_->updateChannel(this);
    // TODO:
}
void Channel::handleEventWithGuard(TimeStamp receiveTime){
    /*
        根据有什么事件执行什么回调操作
        首先 不是if -else的关系 是 if if if 的关系
        先处理错误
        再处理读事件
        最后处理写事件
        先读后写
    */
    //这些奇怪的事件如 epollhup epollerr epollrdhup 等各种事件 在哪注册的呢？

    if(revents_&EPOLLHUP &&!(revents_&EPOLLIN)){
        //连接关闭
        if(closeCallback_){
            closeCallback_();
        }
    }
    if (revents_ & EPOLLERR ){
        //出错
        if (errorCallback_) errorCallback_();
    }
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)){
        // 事件可以检测到对端关闭了写通道，但本地仍然可以读数据
        if (readCallback_) {
            readCallback_(receiveTime);
        }
    }

    if(events_&EPOLLOUT){
        if(writeCallback_){
            writeCallback_();
        }   
    }
}

void Channel::handleEvent(TimeStamp receiveTime){
    /*
        调用tie方法 会让tied_为true
        若tie过 只有被监听的这个 对象还存在 才调用handleEventWithGuard方法执行回调操作
        若不tie 直接调
        为啥只有执行 回调操作才要保证channel存在呢  别的函数不需要连接存在吗？
        什么时候channel不存在呢  是断开连接吗
    */
    //tie何时触发呢 谁调用这个函数去绑定呢？ 帮的是tcpConnection 盲猜是eventLoop
    if(tied_){
        std::shared_ptr<void> guard=tie_.lock();
        if(guard){
            handleEventWithGuard(receiveTime);
        }
    }else{
        handleEventWithGuard(receiveTime);
    }
}


//在eventloop中 将channel删除掉 这个方法谁来调？ 
//我好像还没执行过 删除fd的操作
void Channel::remove(){
    loop_->removeChannel(this);
}

