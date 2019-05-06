//
// Created by user on 19-4-24.
//

#ifndef WEBSERVER_HTTPPARSE_H
#define WEBSERVER_HTTPPARSE_H

#include <linux/aio_abi.h>
#include <sys/socket.h>
#include <cstring>
#include <atomic>

#include "Array.h"
#include "Setting.h"


enum METHOD {

    UNKNOWN,
    GET,
    POST,

    //other temporarily not supported
};

struct HTTPResponse {
    Array *buf;
    iocb *cb;
    int fileLength;
};


struct HTTPRequest {
    std::atomic_llong id;
    //long long id;                     //not thread safe
    //int cgi;
    PoolHeader *pool;
    Array *buf;
    uintptr_t begin;
    int fd;

    METHOD method;
    char *urn_begin;
    char *urn_end;
    char *query_begin;
    char *query_end;
    char *connection;
    char *referer_begin;
    char *referer_end;
    char *accept_begin;
    char *accept_end;
    int content_length;
    char *content_type_begin;
    char *content_type_end;
    char *cookie_begin;
    char *cookie_end;
    char *data_begin;
    char *data_end;

    bool firstLine;
    bool headerEnd;
    bool cgiRes;
    bool jump;

    std::string format;

    HTTPResponse *response;

    void InitVar() {
        id = 0;
        //cgi=-1;
        begin = (uintptr_t) buf->addr;
        method = METHOD::UNKNOWN;
        urn_begin = nullptr;
        urn_end = nullptr;
        query_begin = nullptr;
        query_end = nullptr;
        connection = nullptr;
        referer_begin = nullptr;
        referer_end = nullptr;
        accept_begin = nullptr;
        accept_end = nullptr;
        content_length = 0;
        content_type_begin = nullptr;
        content_type_end = nullptr;
        cookie_begin = nullptr;
        cookie_end = nullptr;
        data_begin = nullptr;
        data_end = nullptr;

        firstLine = false;
        headerEnd = false;
        cgiRes = false;
        jump = false;
    }

    bool init() {
        pool = CreateMemoryPool(MEMORYPOOL_SIZE);
        if (!pool) {
            return false;
        }
        response = (HTTPResponse *) pmalloc(pool, sizeof(HTTPResponse), true);
        response->buf = CreateArray(pool, AIO_SEND_BLOCK_SIZE, 1);
        if (!response->buf) {
            return false;
        }
        buf = CreateArray(pool, ARRAY_RECV_INIT_SIZE, 1);
        if (!buf) {
            return false;
        }
        InitVar();
        return true;
    }

    bool clear() {
        if (buf->capacity > 4095) {
            DestroyArray(buf);
            ResetMemoryPool(pool);
            if (!CreateArray(pool, ARRAY_RECV_INIT_SIZE, 1)) {
                return false;
            }
        }
        ClearArray(response->buf);
        ClearArray(buf);
        return true;
    }

    void reset() {
        id = 0;
        if (!clear()) {
            //return false;
        }
        begin = (uintptr_t) buf->addr;
        InitVar();
    }

    void destroy() {
        DestroyArray(buf);
        DestroyArray(response->buf);
        DestroyMemoryPool(pool);
    }
};

char *find_s(char *buf, const char s[]);

bool CHttpRequestHeader(HTTPRequest *array, int *error);

void sendstatus(int socket, int statuscode);

#endif //WEBSERVER_HTTPPARSE_H
