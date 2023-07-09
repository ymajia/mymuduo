#pragma once
#include <vector>
#include <string>
#include <algorithm>

/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0              readerIndex        writerIndex           size
// 0-readerIndex:           已写且已读的字节
// readerIndex-writerIndex: 已写但未读的字节
// writerIndex-buffer.size: 剩余可写的空间

class Buffer
{
public:
    // 前面预留空间的大小，也是readerIndex和writerIndex初始大小
    // 可以以低廉的代价在头部增加几个字节
    static const size_t kCheapPrepend = 8;
    // 初始writable的大小
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
            : buffer_(kCheapPrepend + initialSize)
            , readerIndex_(kCheapPrepend)
            , writerIndex_(kCheapPrepend) {}

    // 可读的字节 
    size_t readableBytes() const
    {
        return writerIndex_ - readerIndex_;
    }

    // 可写的字节
    size_t writableBytes() const
    {
        return buffer_.size() - writerIndex_;
    }

    // 已读字节
    size_t prependableBytes() const
    {
        return readerIndex_;
    }

    // 返回缓冲区可读数据的起始地址
    const char *peek() const
    {
        return begin() + readerIndex_;
    }

    // 读完数据后执行，判断是否全部读完
    void retrieve(size_t len)
    {
        if (len < readableBytes())
        {
            readerIndex_ += len;
        }
        else
        {
            retrieveAll();
        }
    }

    // 若全部读完则将readerIndex和writerIndex设置为初始值
    void retrieveAll()
    {
        readerIndex_ = writerIndex_ = kCheapPrepend;
    }

    // 把onMessage函数上报的Buffer数据，转成string类型并返回
    std::string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes());   // 应用可读取数据的长度
    }

    std::string retrieveAsString(size_t len)
    {
        std::string result(peek(), len);
        retrieve(len);  // 复位
        return result;
    }

    // 若可写空间不足则需扩容
    void ensureWriteableBytes(size_t len)
    {
        if (writableBytes() < len)
        {
            makeSpace(len); // 扩容
        }
    }

    // 把[data, data + len]上的数据，添加到writeable缓冲区中
    void append(const char *data, size_t len)
    {
        ensureWriteableBytes(len);
        std::copy(data, data + len, beginWrite());
        writerIndex_ += len;
    }

    // read data directly into buffer
    ssize_t readFd(int fd, int *saveErrno);
    // 通过fd发送数据
    ssize_t writeFd(int fd, int *saveErrno);

private:
    char *begin()
    {
        return &*buffer_.begin();   // vector底层数组首元素的地址，即数组的起始地址
    }

    const char *begin() const
    {
        return &*buffer_.begin();
    }

    char *beginWrite()
    {
        return begin() + writerIndex_;
    }

    const char *beginWrite() const
    {
        return begin() + writerIndex_;
    }

    void makeSpace(size_t len)
    {
        if (writableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            buffer_.resize(writerIndex_ + len);
        }
        else
        {
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_, 
                    begin() + writerIndex_,
                    begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }

private:
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};