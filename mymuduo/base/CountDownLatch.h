#pragma once

#include "noncopyable.h"

#include <thread>
#include <mutex>
#include <condition_variable>

class CountDownLatch: public noncopyable
{
public:
    explicit CountDownLatch(int count);
    void wait();
    void countDown();
    int getCount();

private:
    int count_;
    std::mutex mutex_;
    std::condition_variable cond_;
};