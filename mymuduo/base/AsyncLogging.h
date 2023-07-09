#pragma once

#include "noncopyable.h"
#include "LoggerStream.h"
#include "Thread.h"
#include "Timestamp.h"

#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <fstream>
#include <condition_variable>

class AsyncLogging: public noncopyable
{
public:
    AsyncLogging(const std::string &fileName, int flushInterval = 3);
    
    ~AsyncLogging()
    {
        if (isRunning_)
        {
            stop();
        }
    }

    // 提供给前台线程输出日志的接口
    void append(const char *logMsg, size_t len);

    void start();

    void stop();


private:
    void logThreadFunc();

private:
    using Buffer = FixedBuffer<SmallBufferSize>;
    using BufferPtr = std::unique_ptr<Buffer>;
    using BufferVec = std::vector<BufferPtr>;

    std::atomic_bool isRunning_;
    uint32_t flushInterval_;    // 文件flush间隔
    std::string fileName_;
    std::unique_ptr<Thread> logThread_;
    std::mutex mutex_;
    std::condition_variable cond_;

    BufferPtr currBuffer_;
    BufferPtr nextBuffer_;
    BufferVec buffers_;
};