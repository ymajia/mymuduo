// #include <mymuduo/TcpServer.h>
// #include <mymuduo/Logger.h>
#include <TcpServer.h>
#include <Logger.h>
#include <AsyncLogging.h>
#include <string>
#include <functional>
// #include <memory>

class EchoServer
{
public:
    EchoServer(EventLoop *loop, const InetAddress &addr, const std::string &name)
            : server_(loop, addr, name), loop_(loop)
    {
        // 注册回调函数
        server_.setConnectionCallback(
                std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
        server_.setMessageCallback(
                std::bind(&EchoServer::onMessage, this, 
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        
        // 设置合适的线程数量
        server_.setThreadNum(3);
    }

    void start()
    {
        server_.start();
    }

private:
    // 连接建立或者断开的回调
    void onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            // LOG_INFO("Connection Up: %s\n", conn->peerAddress().toIpPort().c_str());
            LOG_INFO << "Connection Up: " << conn->peerAddress().toIpPort();
        }
        else
        {
            // LOG_INFO("Connection Down: %s\n", conn->peerAddress().toIpPort().c_str());
            LOG_INFO << "Connection Down: " << conn->peerAddress().toIpPort();
        }
    }

    // 可读写事件回调
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
    {
        std::string msg = buf->retrieveAllAsString();
        conn->send(msg);
        conn->shutdown();   // 写端，EPOLLHUP => closeCallback_
    }

private:
    EventLoop *loop_;
    TcpServer server_;
};

void startAsyncLogging()
{
    std::string logName = "server";
    static AsyncLogging *asyncLog = new AsyncLogging(logName);
    asyncLog->start();
    Logger::setOutputFunc([](const char *msg, size_t len)
    {
        asyncLog->append(msg, len);
    });
}

int main()
{
    // startAsyncLogging();
    EventLoop loop;
    InetAddress addr(8000);
    EchoServer server(&loop, addr, "EchoServer-01");    // Acceptor non-blocking listenfd create bind
    server.start(); // listen, loopThread, listenFd => acceptChannel => mainLoop =>
    loop.loop();    // 启动mainLoop底层的Poller
    
    return 0;
}