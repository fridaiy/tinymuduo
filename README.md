# tinymuduo

# 1.安装

将项目下载到linux中执行**autobuild.sh**能够自动编译生成动态库，并且将头文件和动态库复制到linux默认的头文件和库的默认路径下。

# 2.Exmaple

使用时只需导入TcpServer的头文件，组装TcpServer和EventLoop对象；向server中注册需要的回调函数;

```c++
//连接到来和关闭时触发的回调
using ConnectionCallback=std::function<void(const TcpConnectionPtr&)>;
//连接关闭时触发的回调
using CloseCallback=std::function<void (const TcpConnectionPtr&)>;
//向客户端发送完数据时触发的回调
using WriteCompleteCallback=std::function<void (const TcpConnectionPtr&)>;
//客户端向服务器发送数据时触发的回调
using MessageCallback=std::function<void (const TcpConnectionPtr&,Buffer*,TimeStamp)>;
```

设置子线程个数;

最后创建EventLoop和TcpServer并且设置服务器的地址；

开启server开启loop;

编译命令：

```sh
 g++ -o 目标文件 源文件.cc -ltinymuduo -lpthread --std=c++14
```

完整示例代码如下:

```c++
#include <iostream>
#include <functional>
#include <memory>
#include <tinymuduo/TcpServer.h>
#include <tinymuduo/Logger.h>

class EchoServer{
public:
    EchoServer(EventLoop *loop,
        const InetAddress& listenAddr,
        const std::string& nameArg)
        :loop_(loop),
        server_(loop,listenAddr,nameArg){
            //注册回调函数注册合适的线程数量
            server_.setMessageCallback(std::bind(&EchoServer::onMessage,this,
            std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));

            server_.setThreadNum(0);

    }
    void start(){
        
        server_.start();
        loop_->loop();
    }
private:
    
    void onMessage(const TcpConnectionPtr &conn,Buffer *buf,TimeStamp timestamp){
        std::string msg=buf->retrieveAllAsString();
        conn->send(msg);
        conn->shutdown();
    }
    EventLoop* loop_;
    TcpServer server_;
};



int main() {
    EventLoop loop;
    InetAddress addr("127.0.0.1",8008);
    const std::string name("echoServer");
    EchoServer echoServer(&loop,addr,name);
    echoServer.start();
  
   

    return 0;
}


```

服务器正常启动:

```sh
[hui@localhost linux]$ ./echo
 _____              ___  ___          _             
|_   _|_            |  \/  |         | |            
  | | (_) _ __ _   _| .  . |_   _  __| |_   _  ___  
  | | | | '_ \| | | | |\/| | | | |/ _` | | | |/ _ \ 
  | | | | | | | |_| | |  | | |_| | (_| | |_| | (_) |
  \_/ |_|_| |_|\__, \_|  |_/\__,_|\__,_|\__,_|\___/ 
                __/ |                               
               |___/                                
[INFO]2024/06/19 12:08:22 : /home/hui/learncpp/mymuduo/Acceptor.cc:listen:37 begin listening
[INFO]2024/06/19 12:08:22 : /home/hui/learncpp/mymuduo/EventLoopThreadPool.ccstart19 EventLoopThreadPool start!
[INFO]2024/06/19 12:08:22 : /home/hui/learncpp/mymuduo/EventLoop.ccloop51 Loop start in threadId_: 28499

```

