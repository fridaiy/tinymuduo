#include "EventLoopThread.h"
#include "Logger.h"
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
        //join 等待子线程结束 EventLoopThread 再完成析构操作
        thread_.join();
    }
}
EventLoop* EventLoopThread::startLoop(){
    thread_.start();//线程创建的EventLoop 所以得有个同步的操作
    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock,[&]()->bool{return loop_!=nullptr;});
        loop = loop_;
    }
    return loop;
}

void EventLoopThread::threadFunc(){
    EventLoop eventLoop;
    if(callback_){
        callback_(&eventLoop);
    }
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_=&eventLoop;
    }
    cv_.notify_all();
    eventLoop.loop();
}