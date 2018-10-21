#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void sigChildFun(int signal) {
    pid_t pid;
    int   stat;
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) { // 循环等待进程结束, 此时waitpid不会阻塞
        NULL;
    }

    return;
}
int main(int argc, char** argv) {
    if (argc < 2) {
        perror("missing parameter!");
        return 1;
    }
    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family      = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;
    servAddr.sin_port        = htons(atoi(argv[1]));

    int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    bind(listen_socket, ( struct sockaddr* )&servAddr, sizeof(servAddr));
    listen(listen_socket, 5);

    signal(SIGCHLD, sigChildFun);

    while (1) {
        struct sockaddr_in clientAddr;
        socklen_t          iSize = sizeof(clientAddr);
        memset(&clientAddr, 0, sizeof(clientAddr));

        int iConnSocket = accept(listen_socket, ( struct sockaddr* )&clientAddr, &iSize);
        if (iConnSocket < 0) {
            if (errno == EINTR || errno == ECONNABORTED) {
                continue;
            }
            else {
                printf("accept error\n");
                return -1;
            }
        }

        int tmpPid = fork();
        if (tmpPid == 0) {
            close(listen_socket); // 子进程让监听socket的计数减1,
                                  // 并非直接关闭监听socket

            char szBuf[1024] = { 0 };
            snprintf(szBuf, sizeof(szBuf), "server pid[%u], client ip[%s]", getpid(), inet_ntoa(clientAddr.sin_addr));
            write(iConnSocket, szBuf, strlen(szBuf) + 1);

            while (1) {
                if (read(iConnSocket, szBuf, 1) <= 0) {
                    close(iConnSocket); // 子进程让通信的socket计数减1
                    return -2;          // 子进程退出
                }
            }

            close(iConnSocket); // 子进程让通信的socket计数减1
            return 0;           // 子进程退出
        }

        close(iConnSocket); // 父进程让通信的socket计数减1
    }

    close(listen_socket); // 父进程让监听socket计数减1,
                          // 此时会关掉监听socket(因为之前子进程已经有此操作)
    return 0;
}
