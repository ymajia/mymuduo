#pragma once

#include <string>
#include <map>
#include <Timestamp.h>

class HttpRequest
{
public:
    using RequestHeaders = std::map<std::string, std::string>;
    enum Method
    {
        MInvalid, MGet, MPost,
    };
    enum Version
    {
        VInvalid, VHttp10, VHttp11,
    };

    HttpRequest(): method_(Method::MInvalid), version_(Version::VInvalid) {}
    ~HttpRequest() = default;
    
    Method getMethod() const 
    {
        return method_;
    }

    Version getVersion() const
    {
        return version_;
    }

    std::string getUrl() const
    {
        return url_;
    }

    std::string getBody() const
    {
        return body_;
    }

    void setMethod(Method method)
    {
        method_ = method;
    }

    void setVersion(Version version)
    {
        version_ = version;
    }

    void setUrl(const std::string &url)
    {
        url_ = url;
    }

    void setBody(const std::string &body)
    {
        body_ = body;
    }

    void addHeaderKV(const std::string &key, const std::string &value)
    {
        headers_[key] = value;
    }

    std::string lookup(const std::string &key) const
    {
        auto findit = headers_.find(key);
        if (findit != headers_.end())
        {
            return findit->second;
        }
        return "";
    }

    const RequestHeaders &getHeaderLines() const
    {
        return headers_;
    }

    void reset()
    {
        method_ = Method::MInvalid;
        version_ = Version::VInvalid;
        url_.clear();
        body_.clear();
        headers_.clear();
    }

private:
    Method method_;
    Version version_;
    std::string url_;
    std::string body_;
    RequestHeaders headers_;

    Timestamp timestamp_;    

};