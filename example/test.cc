#include <iostream>
#include <unordered_map>
#include <string>

// using namespace std;

int main()
{
    // const unordered_map<string, string> map
    // {
    //     {"aa", "a"},
    //     {"bb", "b"},
    //     {"cc", 'c'},
    // };

    const std::unordered_map<std::string, std::string> TypeMap 
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

    std::string type = ".html";
    std::cout << TypeMap.find(type)->second << std::endl;
    // std::cout << TypeMap[type] << std::endl;
    
    return 0;
}