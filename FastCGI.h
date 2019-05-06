//
// Created by user on 19-4-11.
//

#ifndef WEBSERVER_FASTCGI_H
#define WEBSERVER_FASTCGI_H

#include <cstring>

#define FCGI_VERSION 2

enum FCGI_Request_Type {
    FCGI_BEGIN_REQUEST = 1,
    FCGI_ABORT_REQUEST = 2,
    FCGI_END_REQUEST = 3,
    FCGI_PARAMS = 4,
    FCGI_STDIN = 5,
    FCGI_STDOUT = 6,
    FCGI_STDERR = 7,
    FCGI_DATA = 8,
    FCGI_GET_VALUES = 9,
    FCGI_GET_VALUES_RESULT = 10,
    FCGI_UNKNOWN_TYPE = 11
};

enum FCGI_Role {
    FCGI_RESPONDER = 1,
    FCGI_AUTHORIZER = 2,
    FCGI_FILTER = 3
};


struct FCGI_Header {
    unsigned char version;
    unsigned char type;
    unsigned char requestID1;       //high 8 bits
    unsigned char requestID0;       //low
    unsigned char contentLength1;   //
    unsigned char contentLength0;
    unsigned char paddingLength;
    unsigned char reserved;
};

struct FCGI_BeginRequestBody {
    unsigned char role1;
    unsigned char role0;
    unsigned char flags;            //whether close socket    flags&FCGI_KEEP_CONN = 0 close
    unsigned char reserved[5];
};

struct FCGI_EndRequestBody {
    unsigned char appStatus3;
    unsigned char appStatus2;
    unsigned char appStatus1;
    unsigned char appStatus0;
    unsigned char protocolStatus;
    unsigned char reserved[3];
};

struct FCGI_BeginRequest {
    FCGI_Header header;
    FCGI_BeginRequestBody body;
};

struct FCGI_Params {
    FCGI_Header header;
    unsigned char keyLength;
    unsigned char valueLength;
    unsigned char data[0];
};

/*
struct FCGI_Stdin{
    FCGI_Header header;
    unsigned char data[0];
};*/

FCGI_Header makeHeader(int type, int requestID, int contentLength, int paddingLength);

FCGI_BeginRequestBody makeBeginRequestBody(int role);


#endif //WEBSERVER_FASTCGI_H
