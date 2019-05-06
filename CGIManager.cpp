//
// Created by user on 19-4-11.
//

#include "CGIManager.h"
#include "HTTPParse.h"


char *RequestParams::filename = (char *) "SCRIPT_FILENAME";
char *RequestParams::method = (char *) "REQUEST_METHOD";
char *RequestParams::query = (char *) "QUERY_STRING";
char *RequestParams::referer = (char *) "HTTP_REFERER";
char *RequestParams::contentType = (char *) "CONTENT_TYPE";
char *RequestParams::contentLength = (char *) "CONTENT_LENGTH";
char *RequestParams::cookie = (char *) "HTTP_COOKIE";
char *RequestParams::null = (char *) "";

CGIManager::CGIManager() {
    cgifd = -1;
    efd = -1;

    beginRequest.header = makeHeader(FCGI_Request_Type::FCGI_BEGIN_REQUEST, 0, sizeof(beginRequest.body), 0);
    beginRequest.body = makeBeginRequestBody(FCGI_Role::FCGI_RESPONDER);

    strcpy(msg, PATH);

    pool = nullptr;
    responseHeader = nullptr;
    recvPool = nullptr;
    array = nullptr;
    exit = false;
}

CGIManager::~CGIManager() {
    exit = true;
    if (responseHeader != nullptr) {
        pfree(pool, responseHeader);
    }
    if (pool != nullptr) {
        DestroyMemoryPool(pool);
    }
    if (recvPool != nullptr) {
        DestroyMemoryPool(recvPool);
    }
    if (cgifd > 0) {
        close(cgifd);
    }
    if (efd > 0) {
        close(efd);
    }
    listen.join();
}

bool CGIManager::Init() {


    if (!FCGIConnect()) {
        return false;
    }

    pool = CreateMemoryPool(1024);
    if (pool == nullptr) {
        return false;
    }
    responseHeader = (FCGI_Header *) pmalloc(pool, sizeof(FCGI_Header), true);
    if (responseHeader == nullptr) {
        return false;
    }

    recvPool = CreateMemoryPool(4096);
    if (recvPool == nullptr) {
        return false;
    }
    array = CreateArray(recvPool, 1024, 1);
    if (array == nullptr) {
        return false;
    }
    if ((efd = eventfd(0, EFD_NONBLOCK)) < 0) {

        return false;
    }

    BeginRequest(0);

    listen = std::move(std::thread(std::bind(&CGIManager::Response, this)));

    //ParamsReset();

    return true;
}

#include <iostream>

bool CGIManager::BeginRequest(int fd) {
    beginRequest.header.requestID1 = (fd >> 8) & 0xff;
    beginRequest.header.requestID0 = fd & 0xff;

    //std::cout<<int(beginRequest.header.type)<<" "<<int(beginRequest.header.requestID0)<<std::endl;
    if (send(cgifd, (char *) &beginRequest, sizeof(beginRequest), 0) < 0) {
        return false;
    }
    return true;
}


bool CGIManager::Request(HTTPRequest *header) {

    ParamsReset();
    int count = 2;
    if (header->query_end != nullptr) {
        *header->query_end = '\0';
        //*header->referer_end = '\0';
        params[count][0] = RequestParams::query;
        params[count][1] = header->query_begin;
        count++;
    }
    if (header->content_type_end != nullptr) {
        *header->content_type_end = '\0';
        params[count][0] = RequestParams::contentType;
        params[count][1] = header->content_type_begin;

        count++;
    }
    if (header->cookie_end != nullptr) {
        *header->cookie_end = '\0';
        params[count][0] = RequestParams::cookie;
        params[count][1] = header->cookie_begin;
        count++;
    }
    if (header->content_length > 0) {
        sprintf(contentLength, "%d", header->content_length);  //maybe error
        params[count][0] = RequestParams::contentLength;
        params[count][1] = contentLength;
        ++count;
        //std::cout<<contentLength<<std::endl;
    }

    //std::cout<<"start request "<<cgifd<<std::endl;


    strcpy(msg, PATH);
    strcat(msg, std::string(header->urn_begin, header->urn_end).c_str());
    //std::cout << msg << std::endl;
    params[1][1] = header->method == METHOD::GET ? (char *) "GET" : (char *) "POST";
    /*
    char *params[][2] = {{RequestParams::filename, msg},
                         {RequestParams::method, header->method==METHOD::GET?(char*)"GET":(char*)"POST"},
                         {RequestParams::query, header->query_end== nullptr?RequestParams::null:header->query_begin},
                         {RequestParams::contentType,header->content_type_end== nullptr?RequestParams::null:header->cookie_begin},
                         {RequestParams::contentLength,header->content_length>0?contentLength:RequestParams::null},
                         {RequestParams::cookie,header->cookie_end== nullptr?RequestParams::null:header->cookie_begin},
                         {RequestParams::null,     RequestParams::null}};*/
    int len_1, len_2;
    int contentLength, paddingLength;
    FCGI_Params *paramsMsg;


    for (int i = 0; params[i][0] != RequestParams::null; i++) {

        len_1 = strlen(params[i][0]);
        len_2 = strlen(params[i][1]);

        contentLength = len_1 + len_2 + 2;
        paddingLength = (contentLength % 8) == 0 ? 0 : 8 - (contentLength % 8);

        paramsMsg = (FCGI_Params *) pmalloc(pool, sizeof(FCGI_Params) + contentLength + paddingLength, true);
        paramsMsg->keyLength = (unsigned char) len_1;
        paramsMsg->valueLength = (unsigned char) len_2;
        paramsMsg->header = makeHeader(FCGI_Request_Type::FCGI_PARAMS, header->fd, contentLength,
                                       paddingLength);  //optimizable
        //memset(paramsMsg->data, 0, contentLength + paddingLength);
        bzero(paramsMsg->data, contentLength + paddingLength);
        memcpy(paramsMsg->data, params[i][0], len_1);
        memcpy(paramsMsg->data + len_1, params[i][1], len_2);

        if (send(cgifd, (char *) paramsMsg, 8 + contentLength + paddingLength, MSG_DONTWAIT) < 0) {
            return false;
        }

        pfree(pool, paramsMsg);
    }

    paramsMsg = (FCGI_Params *) pmalloc(pool, sizeof(FCGI_Params), true);
    paramsMsg->keyLength = 0;
    paramsMsg->valueLength = 0;
    paramsMsg->header = makeHeader(FCGI_Request_Type::FCGI_PARAMS, header->fd, 0, 0);
    send(cgifd, (char *) paramsMsg, 8, MSG_DONTWAIT);
    pfree(pool, paramsMsg);

    FCGI_Header stdinHeader;
    if (header->data_end) {
        contentLength = header->data_end - header->data_begin;
        paddingLength = (contentLength % 8) == 0 ? 0 : 8 - (contentLength % 8);
        ArrayPush_N(header->buf, paddingLength);
        //FCGI_Stdin *in=(FCGI_Stdin *)pmalloc(pool, sizeof(FCGI_Stdin)+contentLength+paddingLength, true);
        stdinHeader = makeHeader(FCGI_Request_Type::FCGI_STDIN, header->fd, contentLength, paddingLength);
        // memcpy(in->data,header->data_begin,contentLength);
        //std::cout<<contentLength+paddingLength<<std::endl;+contentLength+paddingLength,
        if (send(cgifd, (char *) &stdinHeader, 8, MSG_DONTWAIT) < 0) {
            return false;
        }
        send(cgifd, header->data_begin, contentLength + paddingLength, MSG_DONTWAIT);

    }
    //std::cout<<"send stdin"<<std::endl;
    stdinHeader = makeHeader(FCGI_Request_Type::FCGI_STDIN, header->fd, 0, 0);
    send(cgifd, (char *) &stdinHeader, 8, MSG_DONTWAIT);

    record[header->fd] = header->id.load();
    ClearArray(header->buf);

    //std::cout<<"end request "<<errno<<std::endl;

    return true;
}

bool CGIManager::Response() {

    int contentLength = 0, paddingLength = 0, len = 0, temp = 0;
    int readLength = 0;
    bool normal = false;


    //std::cout << cgifd << std::endl;

    while (!exit) {

        //std::cout<<"thread listen "<<cgifd<<std::endl;
        int len = 0;
        normal = false;
        bzero(responseHeader, sizeof(FCGI_Header));
        if ((len = recv(cgifd, (char *) responseHeader, sizeof(FCGI_Header), 0)) <= 0) {
            //return false;
            std::cout << "thread error " << errno << "-" << len << std::endl;
            close(cgifd);
            FCGIConnect();
            BeginRequest(0);
            Request(&((*map)[0]));
            continue;
        }


        int fd = (int(responseHeader->requestID1) << 8) + int(responseHeader->requestID0);
        if (fd > 0) {
            normal = true;
        }

        ClearArray(array);

        HTTPRequest &header = (*map)[fd];

        contentLength = (int(responseHeader->contentLength1) << 8) + int(responseHeader->contentLength0);
        paddingLength = int(responseHeader->paddingLength);
        len = contentLength + paddingLength;

        temp = len;
        while (temp > 0) {
            if ((readLength = recv(cgifd, ArrayPush_N(array, temp), temp, 0)) <= 0) {
                close(cgifd);
                FCGIConnect();
                continue;
            } else {
                array->used -= (temp - readLength);
                temp -= readLength;
            }

        }

        if (responseHeader->type == FCGI_Request_Type::FCGI_STDOUT ||
            responseHeader->type == FCGI_Request_Type::FCGI_STDERR) {

            if (header.id == record[fd] && normal) {
                if (!header.cgiRes) {
                    write(fd, "HTTP/1.1 200 OK\r\n", 17);
                }
                write(fd, array->addr, readLength);
            }


        } else if (responseHeader->type == FCGI_Request_Type::FCGI_END_REQUEST) {
            //std::cout<<"cgi end "<<int(responseHeader->contentLength0)<<std::endl;
            if (normal && header.id == record[fd]) {
                header.reset();
                epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
                close(fd);
            }
        } else {
            std::cout << "break " << int(responseHeader->type) << std::endl;
            //break;
        }
    }
    std::cout << "thread exit" << std::endl;
    return true;
}

int CGIManager::GetFD() {
    //return cgifd;
    return efd;
}

void CGIManager::SetMonitors(std::unordered_map<int, HTTPRequest> *monitors) {
    map = monitors;
    Request(&((*map)[0]));
}

void CGIManager::SetEpoll(int fd) {
    epfd = fd;
}

bool CGIManager::FCGIConnect() {
    cgifd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (cgifd < 0) {
        std::cout << "cgi fd error " << errno << std::endl;
        return false;
    }

    sockaddr_in cgiaddr;
    char *cgiip = FASTCGI_LISTEN;
    int port = FASTCGI_PORT;

    memset(&cgiaddr, 0, sizeof(cgiaddr));
    cgiaddr.sin_family = AF_INET;
    cgiaddr.sin_addr.s_addr = inet_addr(cgiip);
    cgiaddr.sin_port = htons(port);

    if (connect(cgifd, (sockaddr *) &cgiaddr, sizeof(cgiaddr)) < 0) {
        std::cout << "connect error " << errno << std::endl;
        return false;   //TODO cancel comment
    }
    //fcntl(cgifd, F_SETFL, O_NONBLOCK);           //TODO maybe cancel
    //std::cout << "new cgi fd " << cgifd << std::endl;
    return true;
}

void CGIManager::ParamsReset() {
    params[0][0] = RequestParams::filename;
    params[0][1] = msg;
    params[1][0] = RequestParams::method;
    for (int i = 2; i < 8; ++i) {
        params[i][0] = RequestParams::null;
        params[i][1] = RequestParams::null;
    }
}

