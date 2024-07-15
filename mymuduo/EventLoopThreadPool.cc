#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, 
    const std::string& nameArg)
    :started_(false)
    ,numThreads_(0)
    ,next_(0)
    ,baseLoop_(baseLoop)
    ,name_(nameArg){
        
}
EventLoopThreadPool::~EventLoopThreadPool(){

}

void EventLoopThreadPool::start(const ThreadInitCallback& cb){
    started_=true;
    for (size_t i = 0; i < numThreads_; i++){
    
        auto elt=std::make_unique<EventLoopThread>(cb,std::string(name_+std::to_string(i)));
        loops_.push_back(elt->startLoop());
        threads_.push_back(std::move(elt));
       
    }
    if(numThreads_==0&&cb){
        cb(baseLoop_);
    }
    
}


EventLoop* EventLoopThreadPool::getNextLoop(){
    if(loops_.empty()){
        return baseLoop_;
    }
    next_=(next_+1)%numThreads_;
    return loops_[next_];

}
std::vector<EventLoop*> EventLoopThreadPool::getAllLoops(){
    if(loops_.empty()){
        return std::vector<EventLoop*>(1,baseLoop_);
    }
    return loops_;
}

void EventLoopThreadPool::setThreadNum(int numThreads){
    if(!started_){
        numThreads_=numThreads;
    }
}