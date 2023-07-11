#pragma once

#include <TcpServer.h>
#include <unordered_map>
#include <string>
// #include <Buffer.h>

class EventLoop;
class Buffer;
class InetAddress;

class HttpRequest;
class HttpResponse;

class HttpServer
{
public:
    static const std::string DefualtDir;
    static const std::unordered_map<std::string, std::string> TypeMap;

    // HttpServer(const std::string &name, const std::string &ip, uint32_t port,
    //         size_t ioThreadNum, EventLoop *loop, const std::string &poolName);
    HttpServer(EventLoop *loop, const InetAddress &addr, const std::string &name);
    ~HttpServer() = default;

    void setRecourceDir(std::string resourceDir)
    {
        resourceDir_ = resourceDir;
    }

    void start();

private:
    void onConnect(const TcpConnectionPtr &conn);
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time);
    // 这个接口名称改一下？？
    void makeResponse(const TcpConnectionPtr &conn, const HttpRequest &request);
    void parseUrl(const HttpRequest &req, HttpResponse *resp);

private:
    TcpServer server_;
    std::string resourceDir_;
};