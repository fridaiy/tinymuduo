#include "Thread.h"
#include "CurrentThread.h"
#include "Logger.h"

#include <thread>
#include <mutex>
#include <condition_variable>


Thread::Thread(ThreadFunc func,std::string name):
    func_(std::move(func)),
    name_(name),                                                                                                             
    started_(false),
    joined_(false),
    tid_(0){
        setDefaultName();
}
Thread::~Thread(){
    if(started_&&!joined_){
        thread_->detach();
    }                       
}
void Thread::start(){
    started_=true;
    std::mutex mtxInThreadStart;
    std::condition_variable cvInThreadStart;
    //从这里开始是出现两个线程了 保证start之后 tid是对的
    thread_=std::make_shared<std::thread>(std::thread([&]()->void{
        {
            std::unique_lock<std::mutex> lck(mtxInThreadStart);
            tid_=CurrentThread::tid();
            cvInThreadStart.notify_all();
        }
        func_();

    }));
    LOG_INFO("%s%s%d EventLoopThread start name:  %s tid: %d",__FILE__,__FUNCTION__,__LINE__,name_,tid_);
    std::unique_lock<std::mutex> lck(mtxInThreadStart);
    cvInThreadStart.wait(lck,[&]()->bool{return tid_!=0;});
}
void Thread::join(){
    joined_=true;
    thread_->join();
}

void Thread::setDefaultName(){
    name_="defaultName";
}