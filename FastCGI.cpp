//
// Created by user on 19-4-11.
//

#include "FastCGI.h"

FCGI_Header makeHeader(int type, int requestID, int contentLength, int paddingLength) {
    FCGI_Header header;
    header.version = FCGI_VERSION;
    header.type = type;
    header.requestID1 = (requestID >> 8) & 0xff;
    header.requestID0 = requestID & 0xff;
    header.contentLength1 = (contentLength >> 8) & 0xff;
    header.contentLength0 = contentLength & 0xff;
    header.paddingLength = paddingLength;
    header.reserved = 0;
    return header;
}

FCGI_BeginRequestBody makeBeginRequestBody(int role) {
    FCGI_BeginRequestBody body;
    body.role1 = (role >> 8) & 0xff;
    body.role0 = role & 0xff;
    body.flags = 1;//0xff;
    memset(body.reserved, 0, sizeof(body.reserved));
    return body;
}