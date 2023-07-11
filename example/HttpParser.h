#pragma once

#include "HttpRequest.h"

class Buffer;

class HttpParser
{
public:
    enum ParserState
    {
        ParseRequestLine = 0,
        ParseHeaders,
        ParseBody,
        ParseFinish,
    };

    HttpParser(): state_(ParserState::ParseRequestLine) {}
    ~HttpParser() = default;

    ParserState getState() const
    {
        return state_;
    }

    const HttpRequest &getRequest() const
    {
        return request_;
    }

    bool isFinishAll() const
    {
        return state_ == ParserState::ParseFinish;
    }

    void reset()
    {
        request_.reset();
    }

    bool parsing(Buffer *);

private:
    HttpRequest::Method str2Method(const std::string &mstr);

    void setState(ParserState state)
    {
        state_ = state;
    }

    bool parseRequestLine(const std::string &line); 

private:
    ParserState state_;
    HttpRequest request_;
};