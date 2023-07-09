#include "../AsyncLogging.h"
#include "../Logger.h"
#include "../Timestamp.h"

#include <sys/resource.h>
#include <unistd.h>
#include <stdio.h>
#include <string>

AsyncLogging *asyncLog = nullptr;

void asyncOutput(const char *msg, size_t len)
{
    asyncLog->append(msg, len);
}

void bench(bool longLog)
{
    Logger::setOutputFunc(asyncOutput);
    
    int cnt = 0;
    const int kBatch = 1000;
    std::string empty = "";
    std::string longStr(3000, 'X');
    longStr += " ";

    for (int t = 0; t < 30; t++)
    {
        for (int i = 0; i < kBatch; i++)
        {
            // Logger(__FILE__, __LINE__, __FUNCTION__, Logger::INFO).getStream() << "a";
            LOG_INFO << "Hello 0123456789" << "abcdefghijklmnopqrstuvwxyz";
            cnt++;
        }
        struct timespec ts = { 0, 500*1000*1000 };
        nanosleep(&ts, NULL);
    }

}

int main(int argc, char *argv[])
{
    {
        size_t kOneGB = 1000*1024*1024;
        rlimit rl = { 2*kOneGB, 2*kOneGB };
        setrlimit(RLIMIT_AS, &rl);
    }

    printf("pid = %d\n", getpid());

    char name[256] = { '\0' };
    strncpy(name, argv[0], sizeof(name) - 1);
    AsyncLogging log(::basename(name));
    log.start();
    asyncLog = &log;

    bool longLog = argc > 1;
    bench(longLog);
}
