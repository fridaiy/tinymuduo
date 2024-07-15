#include "Buffer.h"

#include <sys/uio.h>
#include <errno.h>
#include <unistd.h>
ssize_t Buffer::readFd(int fd, int* savedErrno){
    char extrabuf[65536];
    struct iovec vec[2];
    size_t writable=writableBytes();
    vec[0].iov_base=begin()+writerIndex_;
    vec[0].iov_len=writableBytes();
    vec[1].iov_base=extrabuf;
    vec[1].iov_len=65536;
    ssize_t size=readv(fd,vec,2);

    if (size < 0){
        *savedErrno = errno;
    }else if(size<writable){
        writerIndex_+=size;
    }else{
        writerIndex_=buffer_.size();
        append(extrabuf,size-writable);
    }
    return size;
}

ssize_t Buffer::writeFd(int fd, int* saveErrno)
{
    ssize_t n = ::write(fd, peek(), readableBytes());
    if (n < 0)
    {
        *saveErrno = errno;
    }
    return n;
}