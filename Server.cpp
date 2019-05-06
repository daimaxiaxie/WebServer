//
// Created by user on 19-3-19.
//

#include "Server.h"
#include <iostream>

int CreateServer() {
    //hostent *ptrh = nullptr;
    sockaddr_in servaddr;                  //no need serveraddr as function parameter,even if used as pointer in bind()
    int listenfd;
    //int port = 8080;
    int QLEN = 100;

    memset((char *) &servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons((u_short) PORT);

    /*uid_t uid = getuid();
    if (setuid(0))
    {
        perror("");
        return -1;
    }*/
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        std::cout << "SOCKET Creation Failed" << std::endl;
    }
    const socklen_t on = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    //int optval = 1;
    //setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

    if (bind(listenfd, (sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
    }
    if (listen(listenfd, QLEN) < 0) {
        std::cout << "Listen failed" << std::endl;
    }
    std::cout << "Start" << std::endl;
    return listenfd;
}