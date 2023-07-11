#include "HttpResponse.h"

const std::unordered_map<int, std::string> HttpResponse::StatusMap 
{
    {0, "Unknown"},
    {200, "OK"},
    {304, "Moved Permanently"},
    {400, "Bad Request"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {500, "Internal Error"}
};


void HttpResponse::appendToBuffer(Buffer *buf)
{
    // 添加请求行
    char tmpBuf[32];
    snprintf(tmpBuf, sizeof(tmpBuf), "Http/1.1 %d", statusCode_);
    // std::string tmpBuf = "Http/1.1 " + statusCode_;
    buf->append(tmpBuf);
    buf->append(statusMessage_);
    buf->append("\r\n");

    // 添加请求头
    if (isCloseConnection())
    {
        buf->append("Connection: close\r\n");
    }
    else
    {
        // Content-Length设置为body_.size()会导致浏览器迟迟不返回
        snprintf(tmpBuf, sizeof(tmpBuf), "Content-Length: %zd\r\n", body_.size());
        buf->append(tmpBuf);
        buf->append("Connection: Keep-Alive\r\n");
    }
    
    for (auto it = headers_.begin(); it != headers_.end(); ++it)
    {
        buf->append(it->first);
        buf->append(": ", 2);
        buf->append(it->second);
        buf->append("\r\n", 2);
    }

    buf->append("\r\n", 2);
    buf->append(body_);
}