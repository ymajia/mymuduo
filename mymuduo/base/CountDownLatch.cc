#include "CountDownLatch.h"

CountDownLatch::CountDownLatch(int count)
    : cond_()
    , mutex_()
    , count_(count)
{}

void CountDownLatch::wait()
{
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [&] { return count_ <= 0; });
}

void CountDownLatch::countDown()
{
    std::lock_guard<std::mutex> lock(mutex_);
    count_--;
    if (count_ == 0)
    {
        cond_.notify_all();
    }
}

int CountDownLatch::getCount()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return count_;
}