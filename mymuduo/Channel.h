#pragma once

#include "noncopyable.h"
#include "TimeStamp.h"

#include <functional>
#include <memory>

class EventLoop;

class Channel:noncopyable{
public:
    using EventCallback=std::function<void()>;
    using ReadEventCallback=std::function<void(TimeStamp)>;

    Channel(EventLoop* loop, int fd);//只用到EventLoop指针 直接声明就行不用引入头文件
    ~Channel();


    void setReadCallback(ReadEventCallback cb){ readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb){ writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb){ closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb){ errorCallback_ = std::move(cb); }
    //channel得到poller的通知后调用相应的回调方法
    void handleEvent(TimeStamp receiveTime);
    /*
    至少下面这三个函数 是给epollpoller调用的
    */
    void set_revents(int revt) { revents_ = revt; }
    int events() const { return events_; }
    int fd() const { return fd_; }

    /*
    这些函数最终调用的是中的update epollpoller里的 updateChannel
    但是不是epollpoller里的update()
    我前面说过 加入有个增加 fd上的事件的时候 参数不用额外传添加了哪些事件
    而是把channel->events 中注册的事件 同步到epoll内核事件表中
    修改channel.events 是在这进行的
    */
    //设置channel里的事件 注册fd上监听的事件
    void enableReading() { events_ |= kReadEvent; update(); }
    void disableReading() { events_ &= ~kReadEvent; update(); }
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() { events_ &= ~kWriteEvent; update(); }
    void disableAll() { events_ = kNoneEvent; update(); }


    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }
    bool isNoneEvent()const {return events_==kNoneEvent;}

    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }

    EventLoop* ownerLoop() { return loop_; }
    void remove();

    void tie(const std::shared_ptr<void>&);
private:
    void update();
    //上面的handleEvent调用的这个 受保护的handleEvent
    void handleEventWithGuard(TimeStamp receiveTime);
    //目前理解是标记位啊 对感兴趣的事件信息的状态描述
    //我比较好奇这些东西会被初始化成啥 是用epollin那些东西初始化吗
    //果然是 而且是在.cc里初始化 不用再带static
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;//是eventloop包含（多个）channel 但是也得知道这个channel指向的是哪个eventloop
    const int fd_;
    /*
        channel通过events_向epoll中注册感兴趣的事件
        epoll通过revents告诉channel 发生了那些事件 
        然后channel负责调用回调操作
    */
    int events_;
    int revents_;//已发生的事件
    int index_; //后续体会有啥用吧

    /*防止 channel被remove以后还在使用channel
      使用弱智能指针来跨线程监听它
      后续体会吧
     */
    std::weak_ptr<void> tie_;
    bool tied_;

    /*channel 中的revents能够获知fd上发生了哪些事件 因此他负责调用具体的回调操作
    何时 触发回调？
    这些回调具体干什么要用户指定 因此要提供设置回调函数的接口
    */
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};