//
// Created by user on 19-4-24.
//

#include "HTTPParse.h"

const char *buf_404 = "HTTP/1.1 404 Not Found\r\nContemt-Type:text/plain\r\nContent-Length:13\r\n\r\n404 Not Found";
const char *buf_302 = "HTTP/1.1 302 Found\r\nContemt-Type:text/plain\r\nContent-Length:9\r\n\r\n302 Found";
const char *buf_500 = "HTTP/1.1 500 Internal Server Error\r\nContemt-Type:text/plain\r\nContent-Length:20\r\n\r\n500 Internal Server Error";

char *find_s(char *buf, const char s[]) {
    char *c;
    c = buf;
    while ((c = strpbrk(c, ":"))) {
        c--;
        int l = strlen(s) - 1;
        int i = 0;
        while (i < l + 1) {
            if ((*c == s[l - i]) || (*c == (s[l - i] - 32))) {
                c--;
                i++;
            } else {
                break;
            }
        }
        if (i == l + 1) {
            c += (l + 4);
            return c;
        }
        c += l;
    }
    return nullptr;
}

//#include <iostream>
bool CHttpRequestHeader(HTTPRequest *array, int *error) {
    *((char *) array->buf->addr + array->buf->used) = '\0';

    if (array->jump) {
        return true;
    }
    if (!array->urn_begin) {

        if ((array->urn_begin = strpbrk((char *) array->buf->addr, " /"))) {
            array->urn_begin++;
        } else if (array->buf->used < 10) {
            return false;
        } else {
            *(error) = 1;
            array->firstLine = true;
            array->jump = true;
            return true;
        }
        if (!(array->urn_end = strpbrk(array->urn_begin, " ?;"))) {
            if (array->buf->used > 500) {
                *error = 1;
                array->firstLine = true;
                array->jump = true;
                return true;
            }
        }

        if (*((char *) array->buf->addr) == 'G') {
            array->method = METHOD::GET;
        } else if (*((char *) array->buf->addr) == 'P') {
            array->method = METHOD::POST;
        } else {
            *(error)++;
            array->firstLine = true;
            array->jump = true;
            return true;
        }
    }

    if (!array->firstLine && !array->query_end) {
        if (*(array->urn_end) == '?' || *(array->urn_end) == ';') {
            array->query_begin = array->urn_end + 1;
            array->query_end = strpbrk(array->urn_end, " ");
            //*array->query_end='\0';
            array->firstLine = true;
        } else {
            array->query_begin = nullptr;
            array->query_end = nullptr;
        }

    }

    if (!array->headerEnd && !(array->connection)) {
        //array->connection = find_s(array->urn_end, "connection");
    }
    if (!array->headerEnd && !(array->referer_end)) {
        if ((array->referer_begin = find_s(array->urn_end, "referer")) &&
            (array->referer_end = strpbrk(array->referer_begin, "\r\n"))) {
            //*array->referer_end='\0';
            array->firstLine = true;
        }

    }
    if (!array->headerEnd && !(array->accept_end)) {
        if ((array->accept_begin = find_s(array->urn_end, "accept")) &&
            (array->accept_end = strpbrk(array->accept_begin, "\r\n"))) {
            //*array->accept_end = '\0';
            array->firstLine = true;
        }

    }
    if (!array->headerEnd && !(array->cookie_end)) {
        if ((array->cookie_begin = find_s(array->urn_end, "cookie")) &&
            (array->cookie_end = strpbrk(array->cookie_begin, "\r\n"))) {
            //*array->cookie_end='\0';
            array->firstLine = true;
        }
    }

    if (!array->headerEnd && !(array->content_type_end)) {
        if ((array->content_type_begin = find_s(array->urn_end, "Content-Type")) &&
            (array->content_type_end = strpbrk(array->content_type_begin, "\r\n"))) {
            //*array->content_type_end='\0';
            array->firstLine = true;
        }
    }

    //std::cout<<int(*((char*)array->buf->addr+445))<<std::endl;
    //std::cout<<*(array->urn_end-1)<<std::endl;
    if (!array->headerEnd && std::strstr((char *) array->urn_end, "\n\r\n")) {
        array->headerEnd = true;
        array->firstLine = true;
        if (array->method == METHOD::GET) {
            array->jump = true;
            return true;
        } else {
            //return false;
        }
    }
    //std::cout<<array->headerEnd<<std::endl;
    if (array->headerEnd && array->method == METHOD::POST) {
        if ((array->data_begin = find_s(array->urn_end, "content-length"))) {
            array->content_length = atoi(array->data_begin);

            if ((array->data_begin = strstr(array->urn_end, "\n\r\n"))) {
                array->data_begin += 3;
                int now_used = array->buf->used - (array->data_begin - (char *) array->begin);
                if (array->content_length == now_used) {
                    array->data_end = array->data_begin + now_used;

                    return true;
                } else {
                    return false;
                }
            }
        } else if (find_s(array->urn_end, "transfer-encoding")) {
            if ((array->data_begin = strstr(array->urn_end, "\n\r\n"))) {
                array->data_begin += 3;
                if ((array->data_end = strstr(array->urn_end, "\r\n0\r\n"))) {
                    *array->data_end = '\0';
                    return true;
                } else {
                    return false;
                }
            }

            return true;
        } else {
            array->jump = true;
            return true;
        }
    }


    return false;
}

void sendstatus(int socket, int statuscode) {

    switch (statuscode) {
        case 404:
            send(socket, buf_404, strlen(buf_404), 0);
            break;
        case 302:
            send(socket, buf_302, strlen(buf_302), 0);
            break;
        case 500:
            send(socket, buf_500, strlen(buf_500), 0);
            break;
        default:
            break;
    }

}