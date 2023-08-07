#include "EventLoop.h"
#include "Logger.h"
#include "Poller.h"
#include "Channel.h"
#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory>

// 防止一个线程创建多个EventLoop thread_local
__thread EventLoop *t_loopInThisThread = nullptr;

// 定义默认的Poller IO复用接口的超时时间
const int kPollTimeMs = 10000;

// 创建wakeupfd，用来notify唤醒subReactor处理新来的channel
int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG_FATAL << "eventfd error: " << errno;
    }
    return evtfd;
}

EventLoop::EventLoop()
        : looping_(false)
        , quit_(false)
        , callingPendingFunctors_(false)
        , threadId_(CurrentThread::tid())
        , poller_(Poller::newDefaultPoller(this))
        , wakeupFd_(createEventfd())
        , wakeupChannel_(new Channel(this, wakeupFd_))
        // , currentActiveChannel_(nullptr)
{
    // LOG_DEBUG("EventLoop create %p in thread %d\n", this, threadId_);
    LOG_DEBUG << "EventLoop create " << this << " in thread " << threadId_;
    if (t_loopInThisThread)
    {
        LOG_FATAL << "Another EventLoop " << t_loopInThisThread << " exists in this thread " << threadId_;
        // LOG_FATAL("Another EventLoop %p exists in this thread %d\n", t_loopInThisThread, threadId_);
    }
    else
    {
        t_loopInThisThread = this;
    }

    // 设置wakeup的事件类型以及发生事件后的回调操作
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    // 每一个eventloop都将监听wakeupChannel的EPOLLIN读事件
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

// 开启事件循环
// 就是反复调用封装了的epoll_wait，对发生事件的channel调用事先设置好的回调函数
// 最后若设置了pendingFunctors则调用，这组回调由mainloop为subloop设置
// 所以wakeupfd的作用其实就是为了执行这组回调，因为若没有wakeupfd
// 则需要等待事件发生才可以执行到这组回调
void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;

    // LOG_INFO("EventLoop %p start looping\n", this);
    LOG_INFO << "EventLoop " << this << " start looping";

    while (!quit_)
    {
        activeChannels_.clear();
        // 监听两类fd，一种是client的fd，一种是wakeupFd
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        for (Channel *channel: activeChannels_)
        {
            // Poller监听哪些channel发生了事件，上报给EventLoop，通知channel处理相应的事件
            // channel会判断发生事件的类型（其实就是注册事件的类型，然后调用相应的回调函数）
            channel->handleEvent(pollReturnTime_);
        }
        // 执行当前EventLoop事件循环需要处理的回调操作
        // mainLoop事先注册一个回调cb（需要subloop来执行）
        // wakeup subloop后，执行下面的方法，执行之前mainloop注册的cb 
        doPendingFunctors();
    }

    // LOG_INFO("EventLoop %p stop looping.\n", this);
    LOG_INFO << "EventLoop " << this << " stop looping";
    looping_ = false;
}

// 退出事件循环
// 1. loop在自己的线程中调用quit
// 2. 在非loop线程中，调用loop的quit
void EventLoop::quit()
{
    quit_ = true;
    if (!isInLoopThread())
    {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor cb)
{
    if (isInLoopThread())
    {
        // 调用该函数runInLoop的线程和当前loop函数所在线程一样，则直接执行
        cb();
    }
    else
    {
        // 调用该函数runInLoop的线程和当前loop函数所在线程不一样，加入回调队列
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(Functor cb)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }

    // 唤醒相应的，需要执行上面回调操作的loop线程
    // callingPendingFunctors_表示当前loop正在执行回调，但是loop又有了新的回调
    if (!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
}

// 这里的读处理函数只处理wakeupfd的读事件，
// 所以叫handleWakeup也许更合适？？
void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one))
    {
        // LOG_ERROR("EventLoop::handleRead() reads %ld bytes instead of 8", n);
        LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

// 向wakeupFd_写一个数据，wakeupChannel就发生读事件，当前loop线程就会被唤醒
void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one))
    {
        // LOG_ERROR("EventLoop::wakeup() writes %lu bytes instead of 8\n", n);
        LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::updateChannel(Channel *channel)
{
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel)
{
    return poller_->hasChannel(channel);
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);    // 将所有回调函数取出
    }

    for (const Functor &functor: functors)
    {
        functor();  // 执行当前loop需要执行的回调操作
    }

    callingPendingFunctors_ = false;
}