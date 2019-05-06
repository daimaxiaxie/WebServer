//
// Created by user on 19-4-24.
//

#ifndef WEBSERVER_FILEMANAGER_H
#define WEBSERVER_FILEMANAGER_H

#include <filesystem>
#include <sys/sendfile.h>
#include <sys/mman.h>
#include <sys/eventfd.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <linux/aio_abi.h>
#include <unistd.h>
//#include <aio.h>
#include <string>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <unordered_map>

#include "Array.h"
#include "HTTPParse.h"
#include "ThreadPool.h"

void SendFile(long long id, HTTPRequest *header, int infd, int size);

class FileManager {
public:
    FileManager(std::string htdocs);

    ~FileManager();

    bool init();

    bool read(HTTPRequest *header);

    bool isPHP(char *filename, char *end, int *err);

    int getEventFD();

    int io_submit(long n, struct iocb **cb);

    int io_getevents(long min_nr, long nr, io_event *events, timespec *tmo);

    int io_cancel(iocb *cb, io_event *event);

private:
    char *find_format(char *begin, char *end);

private:
    std::string path;
    std::string urn;
    ThreadPool *threadPool;
    std::function<void(long long, HTTPRequest *, int, int)> send;
    //PoolHeader *pool;


    int efd;
    aio_context_t ctx;
    std::unordered_map<std::string, int> record;
};


#endif //WEBSERVER_FILEMANAGER_H
