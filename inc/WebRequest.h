/*
 * Author   : Firat Ozkan
 * Contact  : firaatozkan@gmail.com
*/
#ifndef WEBREQUEST_H
#define WEBREQUEST_H

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

typedef enum
{
    NONE,
    GET,
    POST
} HttpRequestType;

typedef struct 
{
    int* clientFd;
    struct sockaddr_in* clientAddr;
    char* clientReqIndex;
    HttpRequestType clientReqType;
} WebRequest;

typedef struct 
{
    const char* index;
    void (*callback)(WebRequest*);
} WebRequestIndexAndCbPair;

void webRequestServeFile(int clientFd, const char* filePath);

#endif