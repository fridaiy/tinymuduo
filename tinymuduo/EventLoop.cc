#include <sys/eventfd.h>

#include "EventLoop.h"
#include "Logger.h"


__thread EventLoop* t_loopInThisThread = nullptr;

const int kPollTimeMs = 10000;

int createEventfd(){
    int eventfd=::eventfd(0,EFD_NONBLOCK | EFD_CLOEXEC);
    if(eventfd<0){
        LOG_FATAL("createEventFd fail");
    }
    return eventfd;
}

EventLoop::EventLoop()
    :looping_(false),
    quit_(true),
    currentActiveChannel_(nullptr),
    callingPendingFunctors_(false),
    pollReturnTime_(0),
    threadId_(CurrentThread::tid()),
    wakeupFd_(createEventfd()),
    wakeupChannel_(new Channel(this,wakeupFd_)){
        
        poller_=std::unique_ptr<Poller>(Poller::newDefaultPoller(this));
        if (t_loopInThisThread){
            LOG_FATAL("%s%s%d this thread is already having a eventLoop",__FILE__,__FUNCTION__,__LINE__);
        }else{
            t_loopInThisThread=this;
        }
        wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleReadForWakeUp, this));
        wakeupChannel_->enableReading();
}

EventLoop::~EventLoop(){
    /*
        像是poller wakeupChannel
        这些都是智能指针会自己释放
    */
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread=nullptr;
}

void EventLoop::loop(){
    LOG_INFO("%s%s%d Loop start in threadId_: %d",__FILE__,__FUNCTION__,__LINE__,threadId_);
    looping_ = true;
    quit_ = false;  
    while (!quit_){
      activeChannels_.clear();
      pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);

      for (Channel* channel : activeChannels_){
        //这个变量不用也行啊/
        //poll上的事件在这就处理了
        currentActiveChannel_ = channel;
        currentActiveChannel_->handleEvent(pollReturnTime_);
      }
      currentActiveChannel_ = NULL;
      //执行任务队列中的任务 外界传入的 需要本线程做的
      //谁调用的runInLoop?
      doPendingFunctors();
    }
    looping_ = false;
}
//TODO: 谁调呢 EventLoopThread::~EventLoopThread()
void EventLoop::quit(){
    quit_ = true;
    if (!isInLoopThread()){
      wakeup();
    }
}
void EventLoop::runInLoop(Functor cb){
    //怎么感觉是外部 让eventLoop执行某个函数啊
    //如果是当前线程发起的 直接执行
    //如果是别的线程发起的 就在loop中 处理完就绪事件
    //再处理这些事件
    if (isInLoopThread()){
      cb();
    }else{
      queueInLoop(std::move(cb));
    }
}
void EventLoop::queueInLoop(Functor cb){
    {
      std::unique_lock<std::mutex> lock(mutex_);
      //向任务队列中添加任务得获取锁啊
      pendingFunctors_.push_back(std::move(cb));
    }
    
    if (!isInLoopThread() || callingPendingFunctors_){
      //callingPendingFunctors_ 就是说现在正在处理回调呢 然而现在这个任务刚刚加入
      //只能等到下一轮,所以这里是为了唤醒下一轮
      wakeup();
    }
}

void EventLoop::doPendingFunctors(){
  std::vector<Functor> functors;
  callingPendingFunctors_ = true;

  {
    std::unique_lock<std::mutex> lock(mutex_);
    functors.swap(pendingFunctors_);
  }

  for (const Functor& functor : functors){
    functor();
  }
  callingPendingFunctors_ = false;
}



void EventLoop::wakeup(){
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one){
      LOG_ERROR("%s%s%d EventLoop::wakeup() writes bytes instead of 8B",__FILE__,__FUNCTION__,__LINE__,threadId_);
    }
}
void EventLoop::handleReadForWakeUp(){
    //wakeupchannel的专属回调操作啊
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof one);
}
void EventLoop::updateChannel(Channel* channel){
  poller_->updateChannel(channel);
}
void EventLoop::removeChannel(Channel* channel){
  poller_->removeChannel(channel);
}
bool EventLoop::hasChannel(Channel* channel){
  return poller_->hasChannel(channel);
}


   


