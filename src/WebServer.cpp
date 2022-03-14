#include <iostream>
#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <fstream>
#include <memory>
#include "WebServer.h"


WebServer::WebServer(const int& portNum) : port(portNum)
{
    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if(serverFd < 0)
        throw std::runtime_error("Couldn't create socket!");
    
    int opt = 1;
    if(setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw std::runtime_error("Couldn't set socket options!");

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    if(bind(serverFd, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) != 0)
        throw std::runtime_error("Couldn't bind socket!");
}

WebServer::~WebServer()
{
    close(serverFd);
}

void WebServer::run()
{
    if(listen(serverFd, 5) != 0)
        throw std::runtime_error("Server couldn't start listening");

    int clientFd;
    struct sockaddr_in clientAddr;
    socklen_t clientSize = sizeof(clientAddr);

    while(true)
    {
        clientFd = accept(serverFd, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientSize);
        if(clientFd >= 0)
            handleClient(clientFd);
    }
}

void WebServer::handleClient(const int& clientFd)
{
    std::unique_ptr<char> buffer(new char[1024]{0});
    if(recv(clientFd, buffer.get(), 1024, 0) > 0)
    {
        HttpRequestType clientRequest = NONE;
        std::string requestIndex = parseHttpRequest(buffer.get(), clientRequest);

        operateCallbacks(clientFd, clientRequest, requestIndex);
    }
    close(clientFd);
}

std::string WebServer::parseHttpRequest(char* bufPtr, HttpRequestType& requestType)
{
    char* parser = strtok(bufPtr, " ");

    if(strcmp(parser, "GET") == 0)
        requestType = GET;
    else if(strcmp(parser, "POST") == 0)
        requestType = POST;

    parser = strtok(nullptr, " ");
    return std::string(parser);
}

void WebServer::operateCallbacks(const int& clientFd, const HttpRequestType& clientRequestType, const std::string& clientRequestIndex)
{
    if(clientRequestType == GET)
    {
        if(getRequestCallbacks.count(clientRequestIndex) > 0)
            getRequestCallbacks[clientRequestIndex](clientFd);
    }
    else if(clientRequestType == POST)
    {
        if(postRequestCallbacks.count(clientRequestIndex) > 0)
            postRequestCallbacks[clientRequestIndex](clientFd);
    }
    else if(clientRequestType == NONE)
        respondToBadResponse();
}

void WebServer::respondToBadResponse()
{

}

void WebServer::addCustomCallback(const std::string& index, const HttpRequestType& requestType, std::function<void(int)> func)
{
    if(requestType == GET)
    {
        if(getRequestCallbacks.count(index) <= 0)
            getRequestCallbacks.insert(std::pair<std::string, std::function<void(int)>>(index, func));
    }
    else if(requestType == POST)
    {
        if(postRequestCallbacks.count(index) <= 0)
            postRequestCallbacks.insert(std::pair<std::string, std::function<void(int)>>(index, func));
    }
}

void WebServer::serveFile(const int& clientFd, const std::string& filePath)
{
    std::ifstream i;
    i.open(filePath, std::ios::in | std::ios::binary);
    if(i.is_open() == false)
        return;

    std::string header = "HTTP/1.1 200 OK\nContent-Type: text/" + getFileExtension(filePath) + "\nContent-Length: ";
    std::string html{};
    std::string littleBody{};
    char c;
    while(i.eof() == false)
    {
        getline(i, littleBody);
        html += littleBody;
    }
    i.close();
    html = header + std::to_string(html.length()) + "\n\n" + html;
    html[html.length() - 1] = '\0';

    send(clientFd, html.c_str(), html.length(), 0);
}

std::string WebServer::getFileExtension(const std::string& fileName)
{
    return fileName.substr(fileName.find_last_of('.') + 1, fileName.length() - 1);
}