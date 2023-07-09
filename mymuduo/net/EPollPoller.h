#pragma once
#include <vector>
#include <sys/epoll.h>
#include "Poller.h"
#include "Timestamp.h"

class Channel;

class EPollPoller: public Poller
{
public:
    EPollPoller(EventLoop *loop);
    ~EPollPoller() override;

    // 重写积累Poller的抽象方法
    Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;
    void updateChannel(Channel *channel) override;
    void removeChannel(Channel *channel) override;

private:
    // 填写活跃的连接
    void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;
    // 更新channel通道
    void update(int operation, Channel *channel);

private:
    static const int kInitEventListSize = 16;
    using EventList = std::vector<epoll_event>;
    int epollfd_;
    EventList events_;
};