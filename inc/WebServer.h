/*
 * Author   : Firat Ozkan
 * Contact  : firaatozkan@gmail.com
*/
#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "WebRequest.h"

#define CALLBACK_TABLE_SIZE 24

typedef struct 
{
    int serverFd;
    struct sockaddr_in serverAddr;
    WebRequestIndexAndCbPair getRequestCallbackTable[CALLBACK_TABLE_SIZE];
    WebRequestIndexAndCbPair postRequestCallbackTable[CALLBACK_TABLE_SIZE];
} WebServer;

void webServerInit(WebServer* ws, int portNum);

void webServerRun(WebServer* ws);

void webServerClose(WebServer* wb);

void webServerAddCallback(WebServer* wb, const char* index, HttpRequestType requestType, void (*callbackFunc)(WebRequest*));

#endif