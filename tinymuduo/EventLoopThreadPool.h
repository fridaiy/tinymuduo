#pragma once

class EventLoopThread;
class EventLoop;
#include "noncopyable.h"


#include <string>
#include <vector>
#include <functional>
#include <memory>
class EventLoopThreadPool:noncopyable{
    /*
        感觉这个东西就是把 eventLoop+thread给集中管理起来 虽然叫池但是不用支持复用操作
        作用 集中管理 轮询操作

        咋说呢啊 题外话 线程函数里创建的变量 才是线程独有的 线程函数里能访问到的 都相当于外部变量

        集中管理？ 说的太抽象了 主要是通过这个相对于子线程来说的 外部变量 调用子线程
        的eventloop的方法向其中分发任务
    */
public:
    using ThreadInitCallback=std::function<void(EventLoop*)>;
    EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg,int numThreads);
    ~EventLoopThreadPool();
    void setThreadNum(int numThreads);
    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    EventLoop* getNextLoop();
    std::vector<EventLoop*> getAllLoops();

    bool started() const{ return started_; }
    const std::string& name() const{ return name_; }
private:
    bool started_;
    int numThreads_;
    int next_;
    EventLoop* baseLoop_;
    std::string name_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    //有了这个就可以调用子线程中eventloop的方法了啊
    //向子loop中分发任务
    std::vector<EventLoop*> loops_;
};