#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <string>
#include <map>
#include <functional>
#include <arpa/inet.h>

class WebServer
{
public:
    typedef enum
    {
        NONE,
        GET,
        POST
    } HttpRequestType;

private:
    int serverFd, port;
    struct sockaddr_in serverAddr;

    std::map<std::string, std::function<void(int)>> getRequestCallbacks{};
    std::map<std::string, std::function<void(int)>> postRequestCallbacks{};

    void handleClient(const int& clientFd);
    std::string parseHttpRequest(char* bufPtr, HttpRequestType& requestType);
    void operateCallbacks(const int& clientFd, const HttpRequestType& clientRequestType, const std::string& clientRequestIndex);
    void respondToBadResponse();
    static std::string getFileExtension(const std::string& fileName);
public:
    explicit WebServer(const int& portNum = 8080);
    ~WebServer();
    void run();
    void addCustomCallback(const std::string& index, const HttpRequestType& requestType, std::function<void(int)> func);
    void serveFileOn(const std::string& index, const HttpRequestType& requestType, const std::string& filePath);
    static void serveFile(const int& clientFd, const std::string& filePath);
};

std::string getFileExtension(const std::string& fileName);

#endif