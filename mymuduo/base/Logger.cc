#include "Logger.h"
#include "AsyncLogging.h"

std::vector<std::string> Logger::LevelMap = 
{
    "INFO",
    "ERROR",
    "DEBUF",
    "FATAL",
};

void defaultOutput(const char *buf, size_t len)
{
    ::fwrite(buf, 1, len, stdout);
}

Logger::OutputFunc Logger::LogOutputFunc = defaultOutput;

Logger::Logger(const char *fileName, unsigned int lineno,  const char *funcName, LogLevel level)
        : lineno_(lineno)
        , level_(level)
        , stream_()
        , timestamp_(Timestamp::now())
{
    stream_ << "[" << LevelMap[level_] << "]";
    stream_ << "[" << timestamp_.toString() << "]";
    // stream_ << "[tid: " << std::this_thread::get_id() << "]";
    stream_ << "[" << getFileName(fileName) << ":" << lineno;
    stream_ << " " << funcName << "]";
}

Logger::~Logger()
{
    // 析构时调用输出函数
    stream_ << "\n";
    // auto buffer = stream_.getBuffer();
    LogOutputFunc(stream_.getBuffer(), stream_.getBufferLen());
}

void Logger::setOutputFunc(OutputFunc func)
{
    LogOutputFunc = func;
}

std::string Logger::getFileName(const char *name)
{
    const char *slash = strrchr(name, '/');
    if (slash)
    {
        return slash + 1;
    }
    return name;
}