//
// Created by user on 19-3-24.
//

#include "Worker.h"

#include <iostream>

void worker(int listenfd, int ipcfd) {
    std::cout << getpid() << std::endl;

    bool exit = false;
    int count = 0;

    int clientfd;
    socklen_t alen;
    sockaddr_in clientaddr;
    //char buf[1024];

    int epfd = epoll_create(1);
    if (epfd < 0) {
        std::cout << "Epoll create failed" << std::endl;
        perror("");
    }
    epoll_event event, events[10];

    //FastCGI init
    CGIManager cgiManager;
    if (!cgiManager.Init()) {
        //exit= true;
        //return 0;
    }
    int cgifd = cgiManager.GetFD();

    //FileManager init
    FileManager fileManager(PATH);
    if (!fileManager.init()) {
        return;
    }
    int eventfd = fileManager.getEventFD();
    unsigned long long ready;
    io_event ioEvent[10];
    timespec timespec;
    timespec.tv_nsec = 0;
    timespec.tv_sec = 0;

    //Epoll init
    event.data.fd = listenfd;
    //event.events = EPOLLIN;
    event.events = EPOLLIN | EPOLLEXCLUSIVE;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &event);

    event.data.fd = ipcfd;
    event.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, ipcfd, &event);

    event.data.fd = cgifd;
    event.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, cgifd, &event);

    event.data.fd = eventfd;
    event.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, eventfd, &event);


    //Time init
    std::unordered_map<int, HTTPRequest> monitors;
    //std::multimap<std::chrono::milliseconds, int> timerQueue;
    Timer timer;

    timer.TimeUpdate();
    timer.InitID();


    //Function init
    for (int k = 0; k < 65535; ++k) {
        if (!monitors[k].init()) {
            std::cout << "false" << std::endl;
            k--;
        }
        monitors[k].fd = k;
    }

    cgiManager.SetMonitors(&monitors);
    cgiManager.SetEpoll(epfd);

    std::cout << "Working" << std::endl;
    while (!exit) {

        //timer.Start();

        int fds = epoll_wait(epfd, events, 10, -1);
        if (fds < 0) {
            std::cout << "Epoll wait failed(Worker)" << std::endl;
            perror("worker");
            continue;
        }
        for (int i = 0; i < fds; ++i) {
            if (events[i].data.fd == ipcfd) {
                //int len;
                //ioctl(cgifd, SIOCINQ, &len);
                //if (len<4)continue;
                char buf[5];
                //int f;
                //std::cout << "FD Len: " << len << std::endl;
                read(ipcfd, buf, 5);
                if (buf[0] == 0 && buf[1] == 1 && buf[2] == 0 && buf[3] == 1) {
                    exit = true;
                }


            } else if (events[i].data.fd == cgifd) {

                //epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, &events[i]);  //TODO delete this word

                /*int len;
                ioctl(cgifd, SIOCINQ, &len);
                if (len < 8) {
                    continue;
                }*/

                /*
                if (cgiManager.Response()){

                } else{
                    continue;
                }*/

            } else if (events[i].data.fd == listenfd) {

                //std::cout << getpid() << " wakeup" << std::endl;
                alen = sizeof(clientaddr);
                if ((clientfd = accept(listenfd, (sockaddr *) &clientaddr, &alen)) < 0) {
                    std::cout << "Accept failed" << std::endl;
                    perror("");
                    //break;
                    continue;
                }

                monitors[clientfd].id = timer.makeID();

                //std::cout << getpid() << " accept : " << clientfd << std::endl;

                int val = 1;
                fcntl(clientfd, F_SETFL, O_NONBLOCK);
                setsockopt(clientfd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val));
                val = 10;
                setsockopt(clientfd, IPPROTO_TCP, TCP_KEEPIDLE, &val, sizeof(val));
                val = 4;
                setsockopt(clientfd, IPPROTO_TCP, TCP_KEEPINTVL, &val, sizeof(val));
                val = 1;
                setsockopt(clientfd, IPPROTO_TCP, TCP_KEEPCNT, &val, sizeof(val));

                event.data.fd = clientfd;
                event.events = EPOLLIN | EPOLLET;
                if (epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &event) < 0) {
                    perror("Worker : ");
                    epoll_ctl(epfd, EPOLL_CTL_DEL, clientfd, &event);
                    sendstatus(clientfd, 500);
                    close(clientfd);
                    monitors[clientfd].reset();
                    continue;
                }

                //time(&time_now);
                //time_now=timer.GetTime(std::chrono::steady_clock::now());

                //timerQueue.insert(std::make_pair(time_now,clientfd));

                ++count;
                if (count > PROCESS_MAX) {
                    break;
                }

            } else if (events[i].data.fd == eventfd) {
                //std::cout<<"SendFile Callback"<<std::endl;
                if (read(eventfd, &ready, 8) != 8) {
                    continue;
                }

                std::cout << ready << std::endl;
                int n = 0;

                while (ready > 0) {
                    n = fileManager.io_getevents(1, 10, ioEvent, &timespec);
                    if (n > 0) {
                        for (int j = 0; j < n; ++j) {
                            //std::cout<<"file"<<std::endl;
                            int fd = *((int *) ioEvent[j].data);
                            std::cout << fd << ":";
                            std::cout << (char *) monitors[fd].response->buf->addr << std::endl;
                            if (monitors[fd].id == 0) {
                                monitors[fd].reset();
                                continue;
                            }
                            iocb *cb = (iocb *) ioEvent[j].obj;
                            if (cb->aio_offset == 0) {
                                write(fd, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n", 44);
                            }

                            //std::cout<<"res: "<<ioEvent[j].res<<" res2 : "<<ioEvent[j].res2<<std::endl;
                            if (ioEvent[j].res > 0 && ioEvent->res2 == 0) {
                                write(fd, monitors[fd].response->buf->addr, ioEvent[j].res);
                                cb->aio_offset += ioEvent[j].res;
                                if (cb->aio_offset >= monitors[fd].response->fileLength &&
                                    monitors[fd].response->fileLength > 0) {
                                    close(fd);
                                    monitors[fd].reset();
                                    continue;
                                }
                                iocb *cbs[1];
                                cbs[0] = monitors[fd].response->cb;
                                if (fileManager.io_submit(1, cbs) < 0) {
                                    std::cout << "continue submit error" << std::endl;
                                    close(fd);
                                    monitors[fd].reset();
                                }
                            } else {
                                close(fd);
                                monitors[fd].reset();
                            }

                        }
                        ready -= n;
                    } else {
                        break;
                    }
                }
                //std::cout<<"over"<<std::endl;

            } else if (events[i].events == EPOLLIN) {
                //std::cout << getpid() << "Worker Events EPOLLIN" << events[i].data.fd << std::endl;
                int fd = events[i].data.fd;
                if (fd < 0) {
                    std::cout << "error" << std::endl;
                    continue;
                }
                ssize_t readLength = 1;
                HTTPRequest &header = monitors[fd];
                int err = 0;

                while (readLength > 0) {

                    if ((readLength = read(fd, ArrayPush_N(monitors[fd].buf, 512), 512)) < 0) {
                        //std::cout << "read null " <<errno<< std::endl;
                        //perror("read error: ");
                        if (errno == ECONNRESET) {
                            perror("");
                            close(fd);
                            epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &events[i]);

                            //clear_queue(fd);
                            monitors[fd].reset();
                            err = -1;
                            break;
                        } else if (errno == EAGAIN) {
                            monitors[fd].buf->used -= 512;
                            //continue;
                            break;
                        }
                    } else if (readLength == 0) {
                        //std::cout << fd << " read 0" << std::endl;
                        monitors[fd].reset();
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &events[i]);
                        close(fd);
                        err = -1;
                        break;
                    }
                    header.buf->used -= (512 - readLength);
                    //std::cout<<monitors[fd].buf->used<<" "<<readLength<<std::endl;
                }
                //std::cout << getpid() << " recv : " << (char*)monitors[fd].buf->addr << std::endl;


                //std::cout<<"recv end : "<<readLength<<std::endl;
                if (!header.jump && (err == 0)) {
                    err = 0;

                    bool end = CHttpRequestHeader(&monitors[fd], &err);

                    //std::cout<<end<<" "<<monitors[fd].content_length<<std::endl;
                    if (err == 0) {
                        err = 0;
                        bool res = fileManager.isPHP(header.urn_begin, header.urn_end, &err);
                        if (res && end) {
                            //std::cout << "php" << std::endl;
                            if (cgiManager.BeginRequest(fd) && cgiManager.Request(&monitors[fd])) {

                            } else {
                                std::cout << "fpm error" << std::endl;
                                write(fd, "HTTP/1.1 302 OK\r\n\r\nBad Request", 30);
                                monitors[fd].reset();
                                epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &events[i]);
                                close(fd);
                            }

                        } else if (!res && (err == 0) && header.firstLine) {
                            //std::cout<<"not php"<<std::endl;
                            header.jump = true;
                            fileManager.read(&monitors[fd]);

                        } else if (!res && header.firstLine && err) {
                            write(fd, "HTTP/1.1 404 OK\r\n\r\nNOT FOUND", 28);
                            close(fd);
                            monitors[fd].reset();
                        }
                    } else {
                        write(fd, "HTTP/1.1 302 OK\r\n\r\nBad Request", 30);
                        close(fd);
                        monitors[fd].reset();
                    }
                }

                //std::cout<<"EPOLLIN exit"<<std::endl;
            } else if (events[i].events == EPOLLHUP) {

                std::cout << "Delete " << events[i].data.fd << std::endl;
                epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, &events[i]);

            } else {
                int fd = events[i].data.fd;
                std::cout << "Worker other events " << fd << std::endl;

                if (monitors[fd].id > 0) {
                    //clear_queue(fd);
                    monitors[fd].reset();
                }

                close(fd);
                epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &events[i]);

            }
        }


    }

    for (int k = 0; k < 65535; ++k) {
        monitors[k].destroy();
    }

    monitors.clear();
    //timerQueue.clear();

    close(eventfd);
    close(cgifd);
    close(ipcfd);

    close(epfd);

    std::cout << "worker exit" << std::endl;

    std::exit(0);

}

pid_t WorkerManage::CreateWorkers(int nums) {
    for (int i = 0; i < nums; ++i) {
        int fd[2];
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) < 0) {
            --i;
            continue;
        }
        pid_t pid = fork();
        if (pid == 0) {
            this->fd = fd[1];
            close(fd[0]);
            return pid;
        } else if (pid < 0) {
            --i;
            continue;
        } else {
            fcntl(fd[0], F_SETFL, O_NONBLOCK);
            fds.emplace_back(fd[0]);
            close(fd[1]);
        }
    }
    return 1;
}


void WorkerManage::CloseWorkers() {
    char c[] = {0, 1, 0, 1, 0, 1};
    for (auto &f:fds) {
        write(f, c, 6);
        close(f);
    }
}

WorkerManage::WorkerManage() {
    fd = -1;
}

WorkerManage::~WorkerManage() {
    fds.clear();
}

ssize_t WorkerManage::Write(int i, char *buf, size_t size) {
    ssize_t res = write(fds[i], buf, size);
    return res;
}


int WorkerManage::getfd() {
    return fd;
}


ssize_t WorkerManage::send_fd(int i, int fd) {

    //fd=dup(fd);

    msghdr msg;
    iovec iov[1];
    char buf[0];

    iov[0].iov_base = buf;
    iov[0].iov_len = 1;
    msg.msg_name = nullptr;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    cmsghdr cmsg;
    cmsg.cmsg_len = CMSG_LEN(sizeof(int));
    cmsg.cmsg_level = SOL_SOCKET;
    cmsg.cmsg_type = SCM_RIGHTS;
    *(int *) CMSG_DATA(&cmsg) = fd;

    msg.msg_control = &cmsg;

    ssize_t size = sendmsg(fds[i], &msg, 0);
    return size;
}

int WorkerManage::recv_fd() {
    msghdr msg;
    iovec iov[1];
    char buf[0];

    iov[0].iov_base = buf;
    iov[0].iov_len = 1;
    msg.msg_name = nullptr;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    cmsghdr cmsg;

    msg.msg_control = &cmsg;
    msg.msg_controllen = CMSG_LEN(sizeof(int));

    recvmsg(fd, &msg, 0);

    int fd = *(int *) CMSG_DATA(&cmsg);
    return fd;
}


