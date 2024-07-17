#pragma once

#include <unistd.h>
#include <syscall.h>

namespace CurrentThread{
    /*
     __thread修饰的变量 在各个线程中都有其独立的副本，各个线程之间的变量副本互不干扰
    
    在 C++11 及其后的标准中，引入了 thread_local 关键字，
    它提供了与 __thread 类似的功能，并且是标准的一部分：
    thread_local int thread_specific_data;

    不加的话相当于是一个全局变量所有线程共享
    */
    extern __thread int t_cachedTid;
    void cacheTid();
    inline int tid(){
        if(__builtin_expect(t_cachedTid==0,0)){
            cacheTid();
        }
        return t_cachedTid;
    }

}

