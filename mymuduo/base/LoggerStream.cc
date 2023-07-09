#include "LoggerStream.h"

LoggerStream &LoggerStream::operator<<(int val)
{
    appendForInter<int>(val);
    return *this;
}

LoggerStream &LoggerStream::operator<<(uint val)
{
    appendForInter<uint>(val);
    return *this;
}

LoggerStream &LoggerStream::operator<<(int64_t val)
{
    appendForInter<int64_t>(val);
    return *this;
}

LoggerStream &LoggerStream::operator<<(uint64_t val)
{
    appendForInter<uint64_t>(val);
    return *this;
}

LoggerStream &LoggerStream::operator<<(int16_t val)
{
    appendForInter<int>(static_cast<int>(val));
    return *this;
}

LoggerStream &LoggerStream::operator<<(uint16_t val)
{
    appendForInter<uint>(static_cast<uint>(val));
    return *this;
}

LoggerStream &LoggerStream::operator<<(double val)
{
    appendForInter<double>(val);
    return *this;
}

LoggerStream &LoggerStream::operator<<(float val)
{
    appendForInter<float>(val);
    return *this;
}

template<class InterType>
void LoggerStream::appendForInter(InterType val)
{
    std::string str(std::to_string(val));
    append(str.c_str(), str.length());
}