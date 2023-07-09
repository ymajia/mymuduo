#include "AsyncLogging.h"

AsyncLogging::AsyncLogging(const std::string &fileName, int flushInterval)
        : isRunning_(false)
        , flushInterval_(flushInterval)
        , fileName_(fileName)
        // , mutex_()
        // , cond_()
        , logThread_(new Thread(std::bind(&AsyncLogging::logThreadFunc, this)))
        , currBuffer_(new Buffer)
        , nextBuffer_(new Buffer)
{
    buffers_.reserve(16);
}

void AsyncLogging::append(const char *logMsg, size_t len)
{
    // currBuffer_->begin();
    std::unique_lock<std::mutex> lock(mutex_);
    // std::lock_guard<std::mutex> lock(mutex_);
    fflush(stdout);
    if (currBuffer_->getAvail() >= len)
    {
        currBuffer_->append(logMsg, len);
    }
    else
    {
        // 将currBuffer_加入buffers中
        buffers_.push_back(std::move(currBuffer_));
        if (!nextBuffer_)
        {
            nextBuffer_ = std::make_unique<Buffer>();
        }
        currBuffer_ = std::move(nextBuffer_);
        currBuffer_->append(logMsg, len);
        // 通知后台线程交换日志
        cond_.notify_all();
    }
}

void AsyncLogging::start()
{
    if (!isRunning_)
    {
        printf("start thread!\n");
        isRunning_ = true;
        logThread_->start();
    }
}

void AsyncLogging::stop()
{
    if (isRunning_)
    {
        isRunning_ = false;
        logThread_->join();
    }
}

void AsyncLogging::logThreadFunc()
{
    BufferPtr newBuffer1 = std::make_unique<Buffer>();
    BufferPtr newBuffer2 = std::make_unique<Buffer>();
    BufferVec buffersToWrite;
    buffersToWrite.reserve(16);
    std::string opFileName = fileName_ + "_" + Timestamp::now().toString();
    std::ofstream opFile(opFileName.c_str(), std::ios::app);
    // opFile.open(opFileName.c_str(), std::ios::app); // 仅append
    if (!opFile)
    {
        printf("file open fail\n");
    }
    printf("log filename: %s\n", opFileName.c_str());
    while (isRunning_)
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            // buffers为空等待flush间隔
            if (buffers_.empty())
            {
                cond_.wait_for(lock, flushInterval_ * std::chrono::seconds(1), [&]()
                {
                    return !buffers_.empty();
                });
            }
            // 将currBuffer加入buffers中
            buffers_.push_back(std::move(currBuffer_));
            currBuffer_ = std::move(newBuffer1);
            if (!nextBuffer_)
            {
                nextBuffer_ = std::move(newBuffer2);
            }
            // 交换日志缓冲数组
            buffersToWrite.swap(buffers_);
        }

        for (auto &buf: buffersToWrite)
        {
            // 输出buffers中的内容
            opFile << buf->getData();
        }

        if (buffersToWrite.size() > 2)
        {
            buffersToWrite.resize(2);
        }

        if (newBuffer1 == nullptr)
        {
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }

        if (newBuffer2 == nullptr)
        {
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }

        buffersToWrite.clear();
        opFile.flush();
    }

    opFile.flush();
    opFile.close();
}