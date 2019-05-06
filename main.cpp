#include <iostream>
#include <sys/wait.h>

#include "Server.h"
#include "Worker.h"


extern int errno;


int main() {

    bool exit = false;
    bool start = true;

    int listenfd = CreateServer();
    WorkerManage workerManage;

    int workers = 0;


    while (!exit) {

        pid_t pid = workerManage.CreateWorkers(WORKERS - workers);

        if (pid == 0) {

            //worker

            worker(listenfd, workerManage.getfd());

            //exit= true;

            //exit(0);

            return 0;
        } else {

            //master

            //signal(0,SIGIO,input);
            if (start) {
                start = false;
                std::thread thread([&]() {
                    char c;
                    while ((c = getchar()) != 'q') {

                    }
                    exit = true;
                    workerManage.CloseWorkers();
                });
                thread.detach();
            }

            workers = WORKERS;
            int t = 0;
            int status = 0;
            t = waitpid(-1, &status, 0);
            if (t < 0) {
                workers = 0;
            } else {
                workers--;
            }

        }
    }

    close(listenfd);

    std::cout << "exit" << std::endl;

    /*if (setuid(uid)) {
        perror("");
    }*/
    return 0;
}
