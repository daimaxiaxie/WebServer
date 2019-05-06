//
// Created by user on 19-5-6.
//

#include "Setting.h"

const char *LISTEN = "127.0.0.1";

int PORT = 8080;

char *PATH = (char *) "/home/user/www";

int WORKERS = 2;

int PROCESS_MAX = 200000;

int MEMORYPOOL_SIZE = 4096;

int ARRAY_RECV_INIT_SIZE = 1024;

//int ARRAY_SEND_INIT_SIZE=512;

int AIO_SEND_BLOCK_SIZE = 512;

int SENDFILE_MAX = 409600;

char *FASTCGI_LISTEN = (char *) "127.0.0.1";

int FASTCGI_PORT = 9000;

int KEEPALIVE_TIME = 10;