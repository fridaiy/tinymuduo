#pragma once

#include "noncopyable.h"

#include <thread>
#include <memory>
#include <functional>
#include <string>
#include <atomic>
class Thread:noncopyable{
    /*
        封装thread是为了啥 就是为了把thread和func绑定在一起

    */
public:
    using ThreadFunc=std::function<void()>;
    explicit Thread(ThreadFunc func,std::string name);
    ~Thread();
    void start();
    void join();

    
    bool started() const { return started_; }
    pid_t tid() const { return tid_; }
    const std::string& name() const { return name_; }

private:

    void setDefaultName(); 
    bool started_;
    bool joined_;
    //c++中的thread 在创建时就启动
    std::shared_ptr<std::thread> thread_;
    pid_t tid_;
    ThreadFunc func_;
    std::string name_;
};