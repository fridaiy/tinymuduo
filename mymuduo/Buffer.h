#pragma once

#include "noncopyable.h"

#include <vector>
#include <string>
#include <algorithm>

class Buffer:noncopyable{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize=kInitialSize)
        :readerIndex_(kCheapPrepend)
        ,writerIndex_(kCheapPrepend)
        ,buffer_(kCheapPrepend+initialSize){
        
    }
    //这个方法是返回可读得数据大小 是往缓冲区里写入得数据
    size_t readableBytes() const{ return writerIndex_ - readerIndex_; }
    //这个方法返回得是缓冲区内还有多少空间可写 
    size_t writableBytes() const{ return buffer_.size() - writerIndex_; }
    size_t prependableBytes() const{ return readerIndex_; }
    //读缓冲区指针
    const char* peek() const{ return begin() + readerIndex_; }
    //这俩方法是读数据是 更改指针
    void retrieveAll(){
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }
    void retrieve(size_t len){
        if (len < readableBytes()){
            readerIndex_ += len;
        }else{
            retrieveAll();
        }
    }
    //这俩是真读 
    std::string retrieveAllAsString(){
        return retrieveAsString(readableBytes());
    }
    std::string retrieveAsString(size_t len){
        std::string result(peek(), len); 
        retrieve(len);
        return result;
    }
    
    void ensureWritableBytes(size_t len){
        if (writableBytes() < len){
            makeSpace(len);
        }
    }
    void append(const char* data,size_t len){
        ensureWritableBytes(len);
        std::copy(data,data+len,begin()+writerIndex_);
        writerIndex_+=len;
    }
    //读就是从fd中读 往buffer中写
    ssize_t readFd(int fd, int* savedErrno);
    //写就是往fd中写 从buffer中读
    ssize_t writeFd(int fd, int* savedErrno);
private:
    char* begin(){ return &*buffer_.begin(); }
    const char* begin() const{ return &*buffer_.begin(); }
    void makeSpace(size_t len){
        if(buffer_.size()-readableBytes()<len+kCheapPrepend){
            buffer_.resize(writerIndex_+len);
        }else{
            //泛型算法
            std::copy(begin()+readerIndex_,begin()+writerIndex_,begin()+kCheapPrepend);
            readerIndex_=kCheapPrepend;
            writerIndex_=readerIndex_+readableBytes();
        }
    }
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};