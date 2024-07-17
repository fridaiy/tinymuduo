#pragma once


#include <string>
class TimeStamp{
private:
    int64_t  microSecondsSinceEpoch_;
public:
    TimeStamp();
    explicit TimeStamp(int64_t microSecondsSinceEpoch);
    static TimeStamp now();
    std::string toString()const;
};