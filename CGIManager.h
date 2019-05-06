//
// Created by user on 19-4-11.
//

#ifndef WEBSERVER_CGIMANAGER_H
#define WEBSERVER_CGIMANAGER_H

#include <linux/sockios.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unordered_map>
#include <functional>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <thread>
#include <map>

#include "FastCGI.h"
#include "Array.h"
#include "HTTPParse.h"


namespace RequestParams {
    extern char *filename;
    extern char *method;
    extern char *query;
    extern char *referer;
    extern char *contentType;
    extern char *contentLength;
    extern char *cookie;
    extern char *null;
}

class CGIManager {

public:
    CGIManager();

    ~CGIManager();

    bool Init();

    bool BeginRequest(int fd);

    bool Request(HTTPRequest *header);

    bool Response();

    int GetFD();

    void SetMonitors(std::unordered_map<int, HTTPRequest> *monitors);

    void SetEpoll(int fd);

private:

    bool FCGIConnect();

    void ParamsReset();

private:
    int cgifd;
    FCGI_BeginRequest beginRequest;
    char msg[100];
    int efd;

    bool exit;    //not thread safe
    std::map<int, long long> record;
    std::unordered_map<int, HTTPRequest> *map;
    std::thread listen;
    int epfd;

    PoolHeader *pool;
    PoolHeader *recvPool;
    Array *array;
    FCGI_Header *responseHeader;
    char contentLength[10];
    char *params[8][2];

};


#endif //WEBSERVER_CGIMANAGER_H
