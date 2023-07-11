#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <AsyncLogging.h>
#include <Logger.h>

void startAsyncLogging()
{
    std::string logName = "server";
    static AsyncLogging *asyncLog = new AsyncLogging(logName);
    asyncLog->start();
    Logger::setOutputFunc([](const char *msg, size_t len)
    {
        asyncLog->append(msg, len);
    });
}

int main(int argc, char *argv[])
{
    startAsyncLogging();
    EventLoop loop;
    InetAddress addr(8000);
    HttpServer server(&loop, addr, "HttpServer");

    server.start();
    loop.loop();

    return 0;
}