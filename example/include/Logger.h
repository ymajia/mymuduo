#pragma once

#include "noncopyable.h"
#include "LoggerStream.h"
#include "Timestamp.h"

#include <string>
#include <vector>
#include <functional>
#include <stdio.h>

class Logger: public noncopyable
{
public:
    using OutputFunc = std::function<void(const char *, size_t)>;

    enum LogLevel
    {
        INFO = 0,
        ERROR,
        DEBUG,
        FATAL,
    };

    static std::vector<std::string> LevelMap;
    static OutputFunc LogOutputFunc;

    static void setOutputFunc(OutputFunc func);


public:
    Logger(const char *fileName, unsigned int lineno,  const char *funcName, LogLevel level);

    ~Logger();

    LoggerStream &getStream()
    {
        return stream_;
    }

private:
    std::string getFileName(const char *name);

private:
    unsigned int lineno_;
    LogLevel level_;
    LoggerStream stream_;
    Timestamp timestamp_;
};

#define LOG_INFO \
Logger(__FILE__, __LINE__, __FUNCTION__, Logger::INFO).getStream()

#define LOG_ERROR \
Logger(__FILE__, __LINE__, __FUNCTION__, Logger::ERROR).getStream()

#define LOG_DEBUG \
Logger(__FILE__, __LINE__, __FUNCTION__, Logger::DEBUG).getStream()

#define LOG_FATAL \
Logger(__FILE__, __LINE__, __FUNCTION__, Logger::FATAL).getStream()
