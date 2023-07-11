#pragma once

#include <string>
#include <map>
#include <Buffer.h>

class HttpResponse
{
public:
    using ResponseHeaders = std::map<std::string, std::string>;
    enum Version 
    {
        VInvalid, VHttp11,
    };
    enum HttpStatusCode
    {
        Unknown,
        Ok = 200,
        MovedPermantly = 301,
        BadRequest = 400,
        Forbiden = 403,
        NotFound = 404,
        InternalError = 500,
    };

    static const std::unordered_map<int, std::string> StatusMap;

    HttpResponse(): version_(Version::VInvalid), statusCode_(HttpResponse::Unknown) {} 
    explicit HttpResponse(bool close)
        : closeConnection_(close)
        , version_(Version::VInvalid)
        , statusCode_(HttpStatusCode::Unknown) {}
    ~HttpResponse() = default;
    
    void appendToBuffer(Buffer *buf);

    void setVersion(Version version)
    {
        version_ = version;
    }

    void setStatusCode(HttpStatusCode statusCode)
    {
        statusCode_ = statusCode;
    }

    void setStatusMessage(const std::string &msg)
    {
        statusMessage_ = msg;
    }

    void setStatus(HttpStatusCode statusCode)
    {
        setStatusCode(statusCode);
        if (StatusMap.count(statusCode) != 0)
            setStatusMessage(StatusMap.find(statusCode)->second);
    }

    void setBody(const std::string &body)
    {
        body_ = body;
    }

    void setCloseConnection(bool close)
    {
        closeConnection_ = close;
    }

    bool isCloseConnection() const
    {
        return closeConnection_;
    }

    void addHeaderKV(const std::string &key, const std::string &value)
    {
        headers_[key] = value;
    }

    void setContentType(const std::string &type)
    {
        addHeaderKV("Content-Type", type);
    }

private:
    bool closeConnection_;
    Version version_;
    HttpStatusCode statusCode_;
    std::string statusMessage_;
    ResponseHeaders headers_;
    std::string body_;
};