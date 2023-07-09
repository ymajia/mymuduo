#pragma once

#include "noncopyable.h"

#include <string.h>
#include <string>
#include <stdio.h>

static const int SmallBufferSize = 4000;
static const int LargeBufferSize = 4000 * 1000;

template<int SIZE>
class FixedBuffer
{
public:
    FixedBuffer(): curr_(buffer_)
    {
        reset();
    }
    ~FixedBuffer() = default;

    void append(const char *content, size_t len)
    {
        if (getAvail() >= len)
        {
            memcpy(curr_, content, len);
            curr_ += len;
        }
    }

    const char *begin() const
    {
        return buffer_;
    }

    const char *getData() const
    {
        return buffer_;
    }

    const char *end() const
    {
        fflush(stdout);
        return buffer_ + sizeof(buffer_);
    }

    const char *getCurr() const
    {
        return curr_;
    }

    void reset()
    {
        memset(buffer_, 0, sizeof(buffer_));
    }
    
    size_t getAvail() const
    {
        fflush(stdout);
        return end() - getCurr();
    }

    std::string toString() const
    {
        return buffer_;
    }

private:
    char buffer_[SIZE];
    char *curr_;
};

class LoggerStream: public noncopyable
{
public:
    LoggerStream &operator<<(const std::string &str)
    {
        append(str.c_str(), str.length());
        return *this;
    }

    LoggerStream &operator<<(const char *str)
    {
        if (str == nullptr)
        {
            append("(nullptr)", 9);
        }
        else
        {
            append(str, strlen(str));
        }
        return *this;
    }

    LoggerStream &operator<<(char ch)
    {
        append(&ch, 1);
        return *this;
    }

    LoggerStream &operator<<(bool b)
    {
        append(b ? "1" : "0", 1);
        return *this;
    }

    LoggerStream &operator<<(int);

    LoggerStream &operator<<(uint);
    
    LoggerStream &operator<<(int64_t);

    LoggerStream &operator<<(uint64_t);

    LoggerStream &operator<<(int16_t);

    LoggerStream &operator<<(uint16_t);

    LoggerStream &operator<<(double);

    LoggerStream &operator<<(float);

    size_t getBufferLen() const
    {
        return buffer_.getCurr() - buffer_.begin();
    }

    const char *getBuffer() const
    {
        return buffer_.begin();
    }

private:
    template<class InterType>
    void appendForInter(InterType val);

    void append(const char *content, size_t len)
    {
        buffer_.append(content, len);
    }

private:
    FixedBuffer<SmallBufferSize> buffer_;
};