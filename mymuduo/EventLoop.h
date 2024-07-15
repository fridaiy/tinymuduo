#pragma once

#include <vector>
#include <functional>
#include <atomic>
#include <mutex>

#include "Poller.h"
#include "Channel.h"
#include "noncopyable.h"
#include "TimeStamp.h"
#include "CurrentThread.h"

class Poller;
class EventLoop:noncopyable{
    /*
    除了channelList和poller别的现在都不知道干嘛用的
    也没说epoll_wait也就是poll谁调用呢
    */
public:
    //pendingFunctor里存的都是返回值为void 参数类型为空的函数  像不像线程池的任务队列？
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();
    /// Must be called in the same thread as creation of the object.
    void loop();
    /// This is not 100% thread safe, if you call through a raw pointer,
    /// better to call through shared_ptr<EventLoop> for 100% safety.
    void quit();
    /// Time when poll returns, usually means data arrival.
    TimeStamp pollReturnTime() const { return pollReturnTime_; }
    /// Runs callback immediately in the loop thread.
    /// It wakes up the loop, and run the cb.
    /// If in the same loop thread, cb is run within the function.
    /// Safe to call from other threads.
    void runInLoop(Functor cb);
    /// Queues callback in the loop thread.
    /// Runs after finish pooling.
    /// Safe to call from other threads.
    void queueInLoop(Functor cb);
    void wakeup();
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
private:
    //ChannelList 也不是存储所有的Channel
    //就是用来存储 调用epoll_wait后
    //哪些channel上有就绪事件需要处理
    //处理上面的回调操作
    using ChannelList=std::vector<Channel*>;
    //Channel和EventLooop的关系是通过
    //Channel指向的EventLoop

    //EventLoop真正拥有的channel
    //就是 wakeupChannel
   
    void handleRead();  // waked up
    void doPendingFunctors();

    void printActiveChannels() const; // DEBUG




    std::atomic_bool looping_;
    std::atomic_bool quit_;

    const pid_t threadId_;

    TimeStamp pollReturnTime_;
    std::unique_ptr<Poller> poller_;
    
    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;
    Channel *currentActiveChannel_;

    //执行回调不是channel执行的吗
    std::atomic_bool callingPendingFunctors_;
    std::vector<Functor> pendingFunctors_;
    std::mutex mutex_;


  
};