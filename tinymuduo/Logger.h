#pragma once

#include <string>
#include "noncopyable.h"

#define LOG_INFO(format,...)\
    do{\
        Logger &logger=Logger::instance();\
        logger.setLevel(INFO);\
        char buf[1024]={0};\
        snprintf(buf,1024,format,##__VA_ARGS__);\
        logger.log(buf);\
    }while(0)
#define LOG_ERROR(format,...)\
    do{\
        Logger &logger=Logger::instance();\
        logger.setLevel(ERROR);\
        char buf[1024]={0};\
        snprintf(buf,1024,format,##__VA_ARGS__);\
        logger.log(buf);\
    }while(0)
#define LOG_FATAL(format,...)\
    do{\
        Logger &logger=Logger::instance();\
        logger.setLevel(FATAL);\
        char buf[1024]={0};\
        snprintf(buf,1024,format,##__VA_ARGS__);\
        logger.log(buf);\
        exit(0);\
    }while(0)
#define LOG_DEBUG(format,...)\
    do{\
        Logger &logger=Logger::instance();\
        logger.setLevel(DEBUG);\
        char buf[1024]={0};\
        snprintf(buf,1024,format,##__VA_ARGS__);\
        logger.log(buf);\
    }while(0)


enum LoggerLevel{
    INFO,
    ERROR,
    FATAL,
    DEBUG
};
/*
    单例模式
*/
class Logger:noncopyable{
private:
    Logger();
    int level_;
public:
    static Logger& instance();
    void setLevel(int level);
    void log(const char* msg);
};