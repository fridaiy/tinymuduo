#include "EventLoopThread.h"

#include <functional>


EventLoopThread::EventLoopThread(const ThreadInitCallback& cb ,const std::string& name )
    :loop_(nullptr),
    thread_(std::bind(&EventLoopThread::threadFunc,this),name),
    callback_(cb),
    exiting_(false){

}

EventLoopThread::~EventLoopThread(){
    exiting_=true;
    if(loop_!=nullptr){
        loop_->quit();
        thread_.join();
    }
}
EventLoop* EventLoopThread::startLoop(){
    thread_.start();//线程创建的EventLoop 所以得有个同步的操作
    EventLoop* loop = nullptr;
    //没问题loop_是线程函数的外部变量（类似全局变量） 能访问到 但是访问全局变量得加锁啊 
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop_ == nullptr)
        {
            cv_.wait(lock);
        }
        loop = loop_;
    }

    return loop;
}

void EventLoopThread::threadFunc(){
    EventLoop eventLoop;
    callback_(&eventLoop);
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_=&eventLoop;
    }
    
    cv_.notify_all();

    eventLoop.loop();
    
}