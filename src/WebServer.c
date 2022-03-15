#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "WebServer.h"

#ifdef MULTITHREADED
#include <pthread.h>
#endif


struct TempBuffer
{
    WebServer* ws;
    WebRequest* wr;
};

static void* handleClient(void* ptr);
static void parseHttpRequest(char* buffer, WebRequest* client);
static void operateCallbacks(WebServer* ws, WebRequest* client);
static int hash(const char* key);
static void returnBadResponse(int clientFd);

void webServerInit(WebServer* ws, int portNum)
{
    memset(ws, 0, sizeof(WebServer));
    ws->serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if(ws->serverFd < 0)
    {
        printf("Couldn't create socket!\n");
        exit(-1);
    }
    int opt = 1;
    if(setsockopt(ws->serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        printf("Couldn't set socket options!\n");
        exit(-1);
    }
    ws->serverAddr.sin_addr.s_addr = INADDR_ANY;
    ws->serverAddr.sin_port = htons(portNum);
    ws->serverAddr.sin_family = AF_INET;
    if(bind(ws->serverFd, (struct sockaddr*)&ws->serverAddr, sizeof(ws->serverAddr)) != 0)
    {
        printf("Couldn't bind socket!\n");
        exit(-1);
    }
    printf("Succesfully created server!\n");
}

void webServerClose(WebServer* wb)
{
    if(wb)
        close(wb->serverFd);
}

void webServerRun(WebServer* ws)
{
    if(listen(ws->serverFd, 5) != 0)
    {
        printf("Server couldn't start listening!\n");
        exit(-1);
    }
    int clientFd;
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    while(1)
    {
        clientFd = accept(ws->serverFd, (struct sockaddr*)&clientAddr, &clientLen);
        if(clientFd >= 0)
        {
            WebRequest newClient = {&clientFd, &clientAddr, NULL, NONE};
            struct TempBuffer t = {ws, &newClient};
            
#ifndef MULTITHREADED
            handleClient(&t);
#else
            pthread_t thd;
            pthread_create(&thd, NULL, handleClient, &t);
#endif
        }
    }
}

static void* handleClient(void* ptr)
{
    WebServer* ws = ((struct TempBuffer*)ptr)->ws;
    WebRequest* client = ((struct TempBuffer*)ptr)->wr;
    char buffer[1024];
    memset(buffer, 0, 1024);
    if(recv(*(client->clientFd), buffer, 1024, 0) > 0)
    {
        parseHttpRequest(buffer, client);
        operateCallbacks(ws, client);
    }
    close(*(client->clientFd));
    return NULL;
}

static void parseHttpRequest(char* buffer, WebRequest* client)
{
    char* parsedChar = strtok(buffer, " ");

    if(strcmp(parsedChar, "GET") == 0)
        client->clientReqType = GET;
    else if(strcmp(parsedChar, "POST") == 0)
        client->clientReqType = POST;

    parsedChar = strtok(NULL, " ");
    client->clientReqIndex = parsedChar;
}

static void operateCallbacks(WebServer* ws, WebRequest* client)
{
    if(client->clientReqType == GET)
    {
        if(ws->getRequestCallbackTable[hash(client->clientReqIndex)].callback != NULL && 
        strcmp(client->clientReqIndex, ws->getRequestCallbackTable[hash(client->clientReqIndex)].index) == 0)
        {
            ws->getRequestCallbackTable[hash(client->clientReqIndex)].callback(client);
        }
        else
            returnBadResponse(*(client->clientFd));
    }
    else if(client->clientReqType == POST)
    {
        if(ws->postRequestCallbackTable[hash(client->clientReqIndex)].callback != NULL && 
        strcmp(client->clientReqIndex, ws->postRequestCallbackTable[hash(client->clientReqIndex)].index) == 0)
        {
            ws->postRequestCallbackTable[hash(client->clientReqIndex)].callback(client);
        }
        else
            returnBadResponse(*(client->clientFd));
    }
}

static int hash(const char* key)
{
    int result = 0;
    int len = strlen(key);
    for (int i = 0; i < len; i++)
    {
        result = (result + key[i]) % len;
        result = (result * key[i]) % len;
    }
    return result;
}

void webServerAddCallback(WebServer* wb, const char* index, HttpRequestType requestType, void (*callbackFunc)(WebRequest*))
{
    if(requestType == GET)
    {
        if(wb->getRequestCallbackTable[hash(index)].callback == NULL)
        {
            WebRequestIndexAndCbPair pair = {index, callbackFunc};
            wb->getRequestCallbackTable[hash(index)] = pair;
        }
    }
    else if(requestType == POST)
    {
        if(wb->postRequestCallbackTable[hash(index)].callback == NULL)
        {
            WebRequestIndexAndCbPair pair = {index, callbackFunc};
            wb->postRequestCallbackTable[hash(index)] = pair;
        }
    }
}

static char* getFileExtension(const char* str)
{
    char* result = NULL;
    for (char* i = (char*) str; *i != 0; i++)
    {
        if(*i == '.')
            result = i;
    }
    return result + 1;
}

static void returnBadResponse(int clientFd)
{
    char dataBuffer[100] = "HTTP/1.1 404 Not Found\nContent-Type text/plain\nContent-Length:20\n\nBad File Request!\n";
    send(clientFd, dataBuffer, strlen(dataBuffer), 0);
}

void webRequestServeFile(int clientFd, const char* filePath)
{
    FILE* f = fopen(filePath, "r");
    if(f)
    {
        char dataBuffer[2048] = "HTTP/1.1 200 OK\nContent-Type: text/"; //plain\nContent-Length: 12\n\nHello world!";;
        strcat(dataBuffer, getFileExtension(filePath));
        strcat(dataBuffer, "\nContent-Length: ");
        char fileBuffer[1024];
        int fileBufferIndex = 0;
        char c;
        while(!feof(f))
        {
            c = fgetc(f);
            fileBuffer[fileBufferIndex++] = c;
        }
        fileBuffer[fileBufferIndex - 1] = '\0';
        sprintf(&dataBuffer[strlen(dataBuffer)], "%d\n\n%s", fileBufferIndex - 1, fileBuffer);
        send(clientFd, dataBuffer, strlen(dataBuffer), 0);
        fclose(f);
    }
    else
        returnBadResponse(clientFd);
}