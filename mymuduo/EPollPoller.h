#pragma once

#include <vector>
#include <unistd.h>
#include <sys/epoll.h>
#include <errno.h>
#include <string.h>

#include "Poller.h"
#include "TimeStamp.h"
#include "Logger.h"

class EPollPoller:public Poller{
public:
    //其实创建epoll不用传啥人为的参数
    EPollPoller(EventLoop* loop);
    ~EPollPoller() override;

    TimeStamp poll(int timeoutMs, ChannelList* activeChannels) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;

private:
    //最多监听事件的初始大小 重点是初始 就初始化是有用
    static const int kInitEventListSize = 16;

    // 这个函数是用来修改channel发生哪些就绪事件
    // 把就绪事件填写到对应channel 的 revent中
    // 并返回 发生事件的channel的集合
    // 上层操作的是 channel的集合(多了层封装吧) 而不是 epoll_event的集合
    void fillActiveChannels(int numEvents,ChannelList* activeChannels) const;
    //将fd（channel）上注册的事件同步到内核时间表中
    void update(int operation, Channel* channel);

    /*
        明确了  这个就是用来把每次调用epoll_wait接受 作为epoll_wait的参数
        事件发生的 epoll_event集合
        
    */
    using EventList=std::vector<struct epoll_event>;

    int epollfd_;
    /*
        就是用来接收返回值啊  真正监听的哪些事件内核里内核事件表中存一份
        channel中存一份 ok了

    */
    EventList events_;
};