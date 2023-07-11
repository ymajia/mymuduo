#include "HttpParser.h"
#include <Buffer.h>

bool HttpParser::parsing(Buffer *buf)
{
    bool parsing = true;
    bool isOk = true;
    while (parsing)
    {
        // 首先以CRLF为边界，解析出一行数据
        auto crlf = buf->findCRLF();
        if (crlf == nullptr) break; // 找不到 \r\n 了
        std::string line(buf->retrieveUntilAsString(crlf));
        buf->retrieve(2);   // 将多余的crlf跳过

        switch (state_)
        {
        case ParserState::ParseRequestLine:
        {
            bool parseOk = parseRequestLine(line);
            if (parseOk)
            {
                // 状态转移
                setState(ParserState::ParseHeaders);
            }
            else
            {
                isOk = false;
                parsing = false;    // 停止解析
            }
            break;
        }
        case ParserState::ParseHeaders:
        {
            // 根据冒号进行分割
            auto colon = std::find(line.begin(), line.end(), ':');
            if (colon == line.end())
            {
                setState(ParserState::ParseFinish);
            }
            else
            {
                std::string key(line.begin(), colon);
                std::string value(colon + 1, line.end());
                request_.addHeaderKV(key, value);
            }
            break;
        }
        case ParserState::ParseBody:
        {
            setState(ParserState::ParseFinish);
            break;
        }
        default:
        {
            parsing = false;
            break;
        }
        }
    }
    return isOk;
}

HttpRequest::Method HttpParser::str2Method(const std::string &mstr)
{
    if (mstr == "POST")
    {
        return HttpRequest::MPost;
    }
    else if (mstr == "GET")
    {
        return HttpRequest::MGet;
    }
    else
    {
        return HttpRequest::MInvalid;
    }
}

bool HttpParser::parseRequestLine(const std::string &line)
{
    // 以空格为单位逐个分割
    auto it = line.begin();
    for (size_t i = 0; i < 3; i++)
    {
        auto space = std::find(it, line.end(), ' ');
        if (space == line.end() && i != 2)
        {
            return false;
        }
        std::string field(it, space);
        switch (i)
        {
        case 0:
        {
            auto method = str2Method(field);
            if (method == HttpRequest::MInvalid)
            {
                return false;
            }
            request_.setMethod(method);
            break;
        }
        case 1:
        {
            request_.setUrl(field);
            break;
        }
        case 2:
        {
            // 首先判断长度是否合法
            if (field.size() != 8 || std::equal(it, space - 1, "Http/1."))
            {
                return false;
            }
            char lastchar = field.back();
            HttpRequest::Version ver;
            if (lastchar == '1')
            {
                ver = HttpRequest::VHttp11;
            }
            else if (lastchar == '0')
            {
                ver = HttpRequest::VHttp10;
            }
            else
            {
                ver = HttpRequest::VInvalid;
            }
            request_.setVersion(ver);
            break;
        }
        }
        // 跳过空格
        it = space + 1;
    }
    return true;
}