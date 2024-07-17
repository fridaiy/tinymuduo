#pragma once

class noncopyable{
protected:
    noncopyable(const noncopyable&)=delete;
    void operator=(const noncopyable&)=delete;
    noncopyable()=default;
    ~noncopyable()=default;

};