#include "HttpServer.h"
#include "HttpParser.h"
#include "HttpResponse.h"
#include <TcpServer.h>
#include <Logger.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

const std::string HttpServer::DefualtDir = "/root/cpp_projects/mymuduo/example/resources";

const std::unordered_map<std::string, std::string> HttpServer::TypeMap
{
    {".html",   "text/html"},
    {".xml",    "text/xml"},
    {".xhtml",  "application/xhtml+xml"},
    {".txt",    "text/plain"},
    {".rtf",    "application/rtf"},
    {".pdf",    "application/pdf"},
    {".word",   "application/nsword"},
    {".png",    "image/png"},
    {".gif",    "image/gif"},
    {".jpg",    "image/jpeg"},
    {".jpeg",   "image/jpeg"},
    {".au",     "audio/basic"},
    {".mpeg",   "video/mpeg"},
    {".mpg",    "video/mpeg"},
    {".avi",    "video/x-msvideo"},
    {".gz",     "application/x-gzip"},
    {".tar",    "application/x-tar"},
    {".css",    "text/css"},
    {".js",     "text/javascript"},
    {".ico",    "image/x-icon"}
};

HttpServer::HttpServer(EventLoop *loop, const InetAddress &addr, const std::string &name)
        : server_(loop, addr, name)
        , resourceDir_(DefualtDir)
{
    server_.setThreadNum(4);
    server_.setConnectionCallback(std::bind(&HttpServer::onConnect, this, std::placeholders::_1));
    server_.setMessageCallback(std::bind(&HttpServer::onMessage, this, 
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void HttpServer::start()
{
    server_.start();
}

void HttpServer::onConnect(const TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        LOG_INFO << "Connection Up: " << conn->peerAddress().toIpPort();
    }
    else
    {
        LOG_INFO << "Connection Down: " << conn->peerAddress().toIpPort();
    }
}

void HttpServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    HttpParser parser;
    if (!parser.parsing(buf))
    {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }

    if (parser.isFinishAll())
    {
        makeResponse(conn, parser.getRequest());
        parser.reset();
    }
}

void HttpServer::makeResponse(const TcpConnectionPtr &conn, const HttpRequest &request)
{
    const std::string &connection = request.lookup("Connection");
    bool close = (connection == "close") 
            || (request.getVersion() == HttpRequest::VHttp10 && connection != "Keep-Alive");
    HttpResponse response(close);
    // 调用用户设置的回调函数
    // httpCallback_(request, &response);
    parseUrl(request, &response);


    Buffer buf;
    response.appendToBuffer(&buf);
    conn->send(std::string(buf.peek(), buf.readableBytes()));
    if (response.isCloseConnection())
    {
        conn->shutdown();
    }

}

void HttpServer::parseUrl(const HttpRequest &req, HttpResponse *resp)
{
    if (req.getUrl() == "/")
    {
        resp->setStatus(HttpResponse::Ok);
        resp->setContentType("text/html");
        resp->addHeaderKV("Server", "Muduo");
        std::string now = Timestamp::now().toString();
        resp->setBody("<html><head><title>This is title</title></head>"
                      "<body><h1>Hello</h1>Now is " + now +
                      "</body></html>");
    }
    else
    {
        std::string page(req.getUrl());
        // 这里filename可以用char[]表示？？也许会更快点
        std::string filename;
        std::string type;

        // 查看文件后缀名，若没有后缀名默认是.html
        auto it = std::find(page.begin(), page.end(), '.');
        if (it != page.end())
        {
            type = std::string(it, page.end());
            filename = resourceDir_ + page;
        }
        else
        {
            type = ".html";
            filename = resourceDir_ + page + type;
        }

        struct stat fileStat;
        int statRet = stat(filename.c_str(), &fileStat);
        if (statRet == -1)
        {
            if (errno == ENOENT)    // 没有该文件
            {
                resp->setStatus(HttpResponse::NotFound);
                resp->setCloseConnection(true);
                return;
            }
        }
        else if (S_ISDIR(fileStat.st_mode)) // 是个目录
        {
            resp->setStatus(HttpResponse::NotFound);
            resp->setCloseConnection(true);
            return;
        }
        else if (!(fileStat.st_mode & S_IROTH)) // 不可读
        {
            resp->setStatus(HttpResponse::Forbiden);
            resp->setCloseConnection(true);
            return;
        }
        // 接下来要打开文件，然后使用文件映射
        int fd = open(filename.c_str(), O_RDONLY);
        if (fd == -1)
        {
            // 这里可以做其他处理，比如返回一个页面？？
            exit(1);
        }
        // 文件映射
        void *mmRet = mmap(NULL, fileStat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        close(fd);
        if (mmRet == MAP_FAILED)
        {
            // 错误处理
            exit(1);
        }
        char *mmFile = (char *)mmRet;

        resp->setStatus(HttpResponse::Ok);
        resp->setContentType((TypeMap.count(type) == 1 ? TypeMap.find(type)->second : "text/plain"));
        resp->addHeaderKV("Server", "Muduo");
        resp->setBody(mmFile);

        munmap(mmFile, fileStat.st_size);
    }
}