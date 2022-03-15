#include <stdio.h>
#include "WebServer.h"

void denemecb(WebRequest* wr)
{
    webRequestServeFile(*(wr->clientFd), "static/deneme.json");
}

int main(void)
{
    WebServer ws;
    webServerInit(&ws, 8080);
    webServerAddCallback(&ws, "/deneme.json", GET, denemecb);
    webServerRun(&ws);
    return 0;
}