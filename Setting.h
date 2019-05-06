//
// Created by user on 19-5-6.
//

#ifndef WEBSERVER_SETTING_H
#define WEBSERVER_SETTING_H

extern const char *LISTEN;   //not change
extern int PORT;

extern char *PATH;

extern int WORKERS;
extern int PROCESS_MAX;

extern int MEMORYPOOL_SIZE;

extern int ARRAY_RECV_INIT_SIZE;
//extern int ARRAY_SEND_INIT_SIZE;

extern int AIO_SEND_BLOCK_SIZE;
extern int SENDFILE_MAX;      //byte unit

extern char *FASTCGI_LISTEN;
extern int FASTCGI_PORT;

extern int KEEPALIVE_TIME;

#endif //WEBSERVER_SETTING_H
