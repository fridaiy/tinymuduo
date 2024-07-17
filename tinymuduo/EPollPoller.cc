#include "EPollPoller.h"
/*
    继承关系初始化父类成员得用  父类的构造函数啊
    更何况 ownerLoop_还是私有属性怎么可能访问得到
*/

const int kNew=-1;//未被监听过
const int kAdded=1;//正在被监听
const int kDeleted=2;//被监听过 目前没被监听

EPollPoller::EPollPoller(EventLoop* loop)
        :epollfd_(epoll_create1(EPOLL_CLOEXEC)),
         Poller(loop),
         events_(kInitEventListSize){
        
            if(epollfd_<0){
                LOG_FATAL("Epoll create fail %s",__func__);
            }
    
}

EPollPoller::~EPollPoller(){
    close(epollfd_);
}

TimeStamp EPollPoller::poll(int timeoutMs, ChannelList* activeChannels) {
    //第三个参数是最多监听多少个事件
    int numEvents=epoll_wait(epollfd_,&*events_.begin(),static_cast<int>(events_.size()),timeoutMs);
    if (numEvents==0){
    }else if(numEvents>0){
        fillActiveChannels(numEvents,activeChannels);
        if(numEvents==events_.size()){
            //说明此时只监听events_.size()个就绪事件，但是全部就绪了 ，说明最多监听
            //这么多个事件不够用了
            
            events_.resize(2*numEvents);
        }
    }else{
        if(errno!=EINTR){
            LOG_FATAL("epoll_wait error");
        }else{
            LOG_INFO("inter");
        }
    }
    return TimeStamp::now();
    
}

void EPollPoller::updateChannel(Channel* channel){
    //所以说update执行什么操作 取决于events
    //是的 因为这个操作除了events监听的时间为空以外
    //其余要么都是监听态 要么为监听变为监听态
    //要么监听态变为另一种监听态
    //总而言之 只要events不为空 有事件要监听 epoll就会监听它 没有事件需要被监听就暂时不去监听它
    /*
        poller里面的这个channels_啊
        凡是加入过这个epoll的channel 都不进行删除
        就是不管 epoll里面有没有正在注册(监听)这个fd
        只要注册过 不显式remove那就在channel中保留
        对这个channel做什么操作 取决于啥 index?还是events?
    */
    int index=channel->index();
    if(index==kNew||index==kDeleted){
        /*
            就是说 只要channel目前没被监听 就把他监听 无论是否监听过
        */
        if(index==kNew){
            /*
            说明第一次加入呗
            加入map中
            */
            channels_[channel->fd()]=channel;
        }
        update(EPOLL_CTL_ADD,channel);
        channel->set_index(kAdded);
    }else{
        /*
            而正在被监听 要么是暂时不让其被监听
            要么更新监听的事件
        */
        if(channel->isNoneEvent()){
            //暂时不被监听
            update(EPOLL_CTL_DEL,channel);
            channel->set_index(kDeleted);
        }else{
            update(EPOLL_CTL_MOD,channel);
        }
    }
        
}
//从poller中移除channel
void EPollPoller::removeChannel(Channel* channel) {
    /*
        果然是像我上面分析的一样 只有remove才是真正的在Poller中移除channel
        如果正在监听 取消监听 然后在channels_中移除出去
    */
        int index=channel->index();
        if(index==kAdded){
            update(EPOLL_CTL_DEL,channel);
        }
        channels_.erase(channel->fd());
        channel->set_index(kNew);
}
void EPollPoller::fillActiveChannels(int numEvents,ChannelList* activeChannels) const{
    for (size_t i = 0; i < numEvents; ++i){
            Channel* channel=static_cast<Channel*>(events_[i].data.ptr);
            //确实是设置的原生的channel
            channel->set_revents(events_[i].events);
            activeChannels->push_back(channel);
    } 
}
void EPollPoller::update(int operation, Channel* channel){
    epoll_event event;
    memset(&event,0,sizeof(event));
    
    //增加和修改合二为一了？ 增加和修改操作通过channel的接口修改 Channel中的events属性吗
    event.events=channel->events();
    event.data.fd=channel->fd();
    event.data.ptr = channel;
    if(::epoll_ctl(epollfd_,operation,channel->fd(),&event)<0){
        if (operation == EPOLL_CTL_DEL){
             LOG_ERROR("epoll_ctl del error:%d\n", errno);
        }else{
            LOG_FATAL("epoll_ctl add/mod error:%d\n", errno);
        }
    }
    

}