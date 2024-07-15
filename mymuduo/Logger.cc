#include "Logger.h"
#include <iostream>
#include "TimeStamp.h"
Logger& Logger::instance(){
    static Logger logger;
    return logger;
}
void Logger::setLevel(int level){
    level_=level;
}
//日志输出格式 [日志级别] time :msg
void Logger::log(const char *msg){
    if(level_==INFO){
        std::cout<<"[INFO]";
    }else if(level_==ERROR){
        std::cout<<"[ERROR]";
    }else if(level_==FATAL){
        std::cout<<"[FATAL]";
    }else if(level_==DEBUG){
        std::cout<<"[DEBUG]";
    }
    std::cout<<TimeStamp::now().toString()<<" : "<<msg<<std::endl;
}

Logger::Logger(){
    
}
/*
int main(){
    Logger &logger=Logger::instance();
    logger.setLevel(INFO);
    
    LOG_INFO("jhhahaha");
}
*/
