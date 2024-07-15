#pragma once

#include <map>
#include <vector>
#include <unordered_map>

#include "EventLoop.h"
#include "Channel.h"
#include "noncopyable.h"
#include "TimeStamp.h"

class Poller:noncopyable{
public:
    using ChannelList=std::vector<Channel*>;

    Poller(EventLoop* loop);
    virtual ~Poller();

    /// epoll_wait
    /// Must be called in the loop thread.
    virtual TimeStamp poll(int timeoutMs, ChannelList* activeChannels) = 0;

    /// 将channel上的事件同步到epoll内核事件表上
    /// Must be called in the loop thread.
    virtual void updateChannel(Channel* channel) = 0;

    /// 将channel(fd)从内核事件表移除
    /// Must be called in the loop thread.
    virtual void removeChannel(Channel* channel) = 0;

    virtual bool hasChannel(Channel* channel) const;

    static Poller* newDefaultPoller(EventLoop* loop);
    /*
    void assertInLoopThread() const
    {
        ownerLoop_->assertInLoopThread();
    }
    */
    
protected:
    using ChannelMap=std::unordered_map<int,Channel*>;
    //这个东西到底是干嘛的 eventLoop有channelList epollpoller有events_
    //意义是我们的poller 可以根据fd 快速的获取channel
    //poller获取channel要干嘛呀?
    //谁把channel设置到这里呢
    //TODO: 感觉是处理新连接的操作 现在不懂 为啥要有tcpConnection直接 在
    //accptor的handleRead把他注册到poller然后 加入到这里
    //谁往这里添加肯定是poller自己的方法
    //盲猜updateChannel
    //没错就是 然后eventloop调用这个方法
    ChannelMap channels_;

private:
    EventLoop* ownerLoop_;
};