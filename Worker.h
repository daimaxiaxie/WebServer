//
// Created by user on 19-3-24.
//

#ifndef WEBSERVER_WORKER_H
#define WEBSERVER_WORKER_H

#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
//#include <time.h>
#include <unordered_map>
#include <map>

#include "FastCGI.h"
#include "Timer.h"
#include "CGIManager.h"
#include "FileManager.h"
#include "Setting.h"

void worker(int listenfd, int ipcfd);

class WorkerManage {
public:
    WorkerManage();

    ~WorkerManage();

    pid_t CreateWorkers(int nums);

    void CloseWorkers();


    int recv_fd();

    ssize_t Write(int i, char *buf, size_t size);

    ssize_t send_fd(int i, int fd);

    int getfd();

private:
    int fd;
    std::vector<int> fds;
};


#endif //WEBSERVER_WORKER_H
