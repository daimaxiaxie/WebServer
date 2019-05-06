//
// Created by user on 19-4-24.
//
#include <iostream>
#include "FileManager.h"
#include "HTTPParse.h"

extern int errno;

void SendFile(long long id, HTTPRequest *header, int infd, int size) {
    //auto s=std::chrono::steady_clock::now();

    //no tcp_cork
    write(header->fd, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n", 44);

    ssize_t len = sendfile(header->fd, infd, nullptr, size);

    //auto e=std::chrono::steady_clock::now();
    //auto d=std::chrono::duration_cast<std::chrono::microseconds>(e-s);
    //std::cout<<"thread time : "<<d.count()<<std::endl;
    //std::cout<<"Sendfile : "<<len<<std::endl;

    close(infd);
    if (header->id == id) {
        close(header->fd);
        header->reset();
    }
}

FileManager::FileManager(std::string htdocs) {
    path = htdocs;
    threadPool = new ThreadPool(2);
}

FileManager::~FileManager() {
    delete threadPool;

    close(efd);
    syscall(SYS_io_destroy, ctx);
}

bool FileManager::init() {
    //std::cout<<"File init 0 "<<std::filesystem::exists("/index.html")<<std::endl;
    //std::filesystem::exists(path + "/index.html")
    //EFD_CLOEXEC
    if ((efd = eventfd(0, EFD_NONBLOCK)) < 0) {

        return false;
    }
    //efd=syscall(SYS_eventfd,0);
    //fcntl(efd,F_SETFL,O_NONBLOCK);
    ctx = 0;
    if (syscall(SYS_io_setup, 10, &ctx) != 0) {
        return false;
    }

    send = [](long long id, HTTPRequest *header, int fd, int size) {

        //auto s=std::chrono::steady_clock::now();

        //no tcp_cork
        write(header->fd, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n", 44);
        //filestat.st_size
        ssize_t len = sendfile(header->fd, fd, nullptr, size);

        //auto e=std::chrono::steady_clock::now();
        //auto d=std::chrono::duration_cast<std::chrono::microseconds>(e-s);
        if (len < 0) {
            perror("sendfile(): ");
            std::cout << header->fd << " " << fd << "-" << size << std::endl;
        }
        //std::cout<<"thread time : "<<d.count()<<std::endl;
        //std::cout<<"Sendfile : "<<len<<std::endl;

        close(fd);
        if (header->id == id) {
            close(header->fd);
            header->reset();
        }

    };

    return true;
}

bool FileManager::read(HTTPRequest *header) {
    std::string str(header->urn_begin, header->urn_end);
    //std::cout << str << std::endl;      //TODO delete
    urn.clear();
    urn = path + str;
    if (*(header->urn_end - 1) == '/') {
        urn += "index";
    }
    if (!find_format(header->urn_begin, header->urn_end)) {
        urn += ".html";
    }

    //O_DIRECT
    int fd = open(urn.c_str(), O_RDONLY);
    if (fd < 0) {
        std::cout << "file open failed " << urn << std::endl;
        perror("error");
        return false;
    }
    //std::cout<<"file: "<<fd<<std::endl;

    struct stat filestat;
    if (fstat(fd, &filestat) < 0) {
        header->response->fileLength = 0;
        return false;
    } else if (filestat.st_size > 0) {
        header->response->fileLength = filestat.st_size;
    }


    if (filestat.st_size < SENDFILE_MAX) {

        //auto s=std::chrono::steady_clock::now();
        //std::cout<<"Begin id: "<<header->id<<" fd: "<<header->fd<<std::endl;
        threadPool->commit(send, header->id.load(), header, fd, filestat.st_size);
        //threadPool->commit(SendFile,header->id,header,fd,filestat.st_size);
        //std::cout<<"process id: "<<header->id<<" fd: "<<header->fd<<std::endl;

        //auto e=std::chrono::steady_clock::now();
        //auto d=std::chrono::duration_cast<std::chrono::microseconds>(e-s);
        //std::cout<<"time : "<<d.count()<<std::endl;
        return true;

    } else {

        iocb *cb = header->response->cb;
        if (!cb) {
            header->response->cb = (iocb *) pmalloc(header->pool, sizeof(iocb), true);
            cb = header->response->cb;
            if (!cb) {
                std::cout << "iocb malloc failed" << std::endl;
                return false;
            }
        }

        bzero(cb, sizeof(iocb));
        cb->aio_lio_opcode = IOCB_CMD_PREAD;
        cb->aio_fildes = fd;
        cb->aio_buf = (uintptr_t) header->response->buf->addr;
        //cb->aio_buf=(uintptr_t)buf;
        cb->aio_nbytes = AIO_SEND_BLOCK_SIZE;
        cb->aio_offset = 0;
        cb->aio_reqprio = 0;
        cb->aio_data = (uintptr_t) &header->fd;
        //std::cout<<*((int*) cb->aio_data)<<std::endl;
        cb->aio_flags = IOCB_FLAG_RESFD;
        cb->aio_resfd = efd;
        //std::cout << "File submit " <<ctx<<" "<<efd<<" "<<errno<< std::endl;
        iocb *cbs[1];
        cbs[0] = header->response->cb;
        int err = syscall(SYS_io_submit, ctx, 1, cbs);
        if (err < 0) {
            //perror("io_submit");
            std::cout << errno << std::endl;
            std::cout << EAGAIN << " " << EBADF << " " << EFAULT << " " << EINVAL << " " << ENOSYS << " " << EPERM
                      << std::endl;
            return false;
        }
        //std::cout << "File ok " <<errno<< std::endl;

        return true;
    }
    return false;
}

bool FileManager::isPHP(char *filename, char *end, int *err) {
    urn.clear();
    std::string str(filename, end);
    std::unordered_map<std::string, int>::iterator it = record.find(str);
    if (it != record.end() && it->second == -1) {
        *err = 1;
        return false;
    }
    if (it != record.end()) {
        return it->second;
    }
    urn = path + str;
    if (*(end - 1) == '/') {
        urn += "index";
    }
    char *f = find_format(filename, end);
    if (!f) {
        if (access((urn + ".html").c_str(), F_OK) == 0) {
            record[str] = 0;
            return false;
        } else if (access((urn + ".php").c_str(), F_OK) == 0) {
            record[str] = 1;
            return true;
        } else {
            *err = -1;
            record[str] = -1;
            return false;
        }
    }
    if (access(urn.c_str(), F_OK) == 0) {
        //std::cout<<(urn.substr(urn.size()-4)==".php")<<std::endl;
        if (urn.substr(urn.size() - 4) == ".php") {
            record[str] = 1;
            return true;
        } else {
            record[str] = 0;
            return false;
        }
    } else {
        *err = -1;
        record[str] = -1;
    }
    return false;
}

int FileManager::getEventFD() {
    return efd;
}

int FileManager::io_submit(long n, struct iocb **cb) {
    return syscall(SYS_io_submit, ctx, n, cb);
}

int FileManager::io_getevents(long min_nr, long nr, io_event *events, timespec *tmo) {
    return syscall(SYS_io_getevents, ctx, min_nr, nr, events, tmo);
}

int FileManager::io_cancel(iocb *cb, io_event *event) {
    return syscall(SYS_io_cancel, cb, event);
}

char *FileManager::find_format(char *begin, char *end) {
    char *t = end;
    for (size_t i = 0; begin < t; i++, t--) {
        if (*t == '.') {
            return t;
        } else if (i > 6) {
            break;
        }
    }
    return nullptr;
}
