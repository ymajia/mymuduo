#include "Acceptor.h"
#include "Logger.h"
#include "InetAddress.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

static int createNonblocking()
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sockfd < 0)
    {
        // LOG_FATAL("%s:%s:%d listen socket create err: %d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        LOG_FATAL << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ 
                << " listen socket create err: " << errno;
    }
    return sockfd;
}

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport)
        : loop_(loop)
        , acceptSocket_(createNonblocking())
        , acceptChannel_(loop, acceptSocket_.fd())
        , listenning_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(true);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceptor::listen()
{
    listenning_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading(); // acceptChannel_ => Poller
}

// listenfd有事件发生，即有新用户连接
void Acceptor::handleRead()
{
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(peerAddr);
    if (connfd >= 0)
    {
        if (newConnectionCallback_)
        {
            // 轮询找到subLoop，唤醒并分发当前的新客户端的Channel
            newConnectionCallback_(connfd, peerAddr);
        }
        else
        {
            ::close(connfd);
        }
    }
    else
    {
        // LOG_ERROR("%s:%s:%d accept err: %d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        LOG_ERROR << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ 
                << " accept err: " << errno;
        if (errno == EMFILE)
        {
            // LOG_ERROR("%s:%s:%d sockfd reached limit err: %d\n", __FILE__, __FUNCTION__, __LINE__, errno);
            LOG_ERROR << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ 
                << " sockfd reached limit err: " << errno;
        }
    }
}