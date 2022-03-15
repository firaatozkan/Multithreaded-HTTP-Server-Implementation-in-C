/*
 * Author   : Firat Ozkan
 * Contact  : firaatozkan@gmail.com
*/
#include <stdio.h>
#include "WebServer.h"

void testCallback(WebRequest* wr)
{
    webRequestServeFile(*(wr->clientFd), "static/test.json");
}

int main(void)
{
    WebServer ws;
    webServerInit(&ws, 8080);
    webServerAddCallback(&ws, "/test.json", GET, testCallback);
    webServerRun(&ws);
    return 0;
}