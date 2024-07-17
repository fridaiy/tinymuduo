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
public:
    //pendingFunctor里存的都是返回值为void 参数类型为空的函数  像不像线程池的任务队列？
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();
    void loop();
    void quit();
    TimeStamp pollReturnTime() const { return pollReturnTime_; }
    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);
    void wakeup();
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);
    
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
private:
    //就是用来存储 调用epoll_wait后
    //哪些channel上有就绪事件需要处理
    //处理上面的回调操作
    using ChannelList=std::vector<Channel*>;
    //Channel和EventLoop的关系是通过
    //Channel指向的EventLoop

    //EventLoop真正拥有的channel
    //就是 wakeupChannel
   
    
    void doPendingFunctors();

    void printActiveChannels() const; // DEBUG




    std::atomic_bool looping_;
    std::atomic_bool quit_;

    const pid_t threadId_;

    TimeStamp pollReturnTime_;
    std::unique_ptr<Poller> poller_;
    
    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;
    void handleReadForWakeUp();

    ChannelList activeChannels_;
    Channel *currentActiveChannel_;

    //执行回调不是channel执行的吗
    std::atomic_bool callingPendingFunctors_;
    std::vector<Functor> pendingFunctors_;
    std::mutex mutex_;


  
};