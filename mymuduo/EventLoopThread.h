#pragma once
#include "Thread.h"
#include "EventLoop.h"

#include <functional>
#include <string>
#include <mutex>
#include <condition_variable>
class EventLoopThread{
public:
    /*
    创建完EventLoop需要进行一些初始化操作的回调 
    (现在感觉回调有好处了 如果不用回调 创建完了写一些初始化操作也行)
    但是这样就写的固定了 想修改得改源码 用回调传入函数的话 调用的层可以 指定方法
    */
    using ThreadInitCallback=std::function<void(EventLoop*)>;
    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                  const std::string& name = std::string());
    ~EventLoopThread();
    EventLoop* startLoop();
private:
    void threadFunc();

    EventLoop* loop_ ;
    bool exiting_;
    Thread thread_;

    /*
        这里使用的mutex 是因为子线程要访问外部变量
    */
    std::mutex mutex_;
    std::condition_variable cv_;

    ThreadInitCallback callback_;

};