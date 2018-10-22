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

#define MAX_MESSAGE_LEN 200

void sigChildFun(int sig) {
    pid_t pid;
    int   stat;
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
    } // 避免僵尸进程

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
    printf("server ready.\n\n");

    while (1) {
        struct sockaddr_in clientAddr;
        socklen_t          iSize = sizeof(clientAddr);
        memset(&clientAddr, 0, sizeof(clientAddr));

        int conn_socket = accept(listen_socket, ( struct sockaddr* )&clientAddr, &iSize);
        if (conn_socket < 0) {
            if (errno == EINTR || errno == ECONNABORTED) {
                continue;
            }
            else {
                printf("accept error\n");
                return -1;
            }
        }
        else {
            printf("new connection from [%s]\n",inet_ntoa(clientAddr.sin_addr));
        }


        int tmpPid = fork();
        if (tmpPid == 0) {
            close(listen_socket); // 子进程让监听socket的计数减1,
                                  // 并非直接关闭监听socket

            char buf[MAX_MESSAGE_LEN] = { 0 };
            snprintf(buf, sizeof(buf), "server pid[%u], client ip[%s]", getpid(), inet_ntoa(clientAddr.sin_addr));
            write(conn_socket, buf, strlen(buf) + 1);

            int ret = 0;
            while (1) {
                bzero(buf, sizeof(buf));
                ret = ( int )read(conn_socket, buf, sizeof(buf));
                buf[ret-1]='\0';
                if (ret < 0) {
                    perror("read error");
                    printf("socket %d closed, child process exit\n", conn_socket);
                    close(conn_socket); // 子进程让通信的socket计数减1
                    return -2;          // 子进程退出
                }
                else {
                    if (strncmp("exit", buf, 4) == 0) {
                        printf("receive exit from client, close child process\n");
                        break;
                    }
                    printf("get message \"%s\"\n", buf);
                    send(conn_socket, buf, sizeof(buf), 0);
                }
            }

            close(conn_socket); // 子进程让通信的socket计数减1
            return 0;           // 子进程退出
        }
        else {
            printf("new connection.");
        }

        close(conn_socket); // 父进程让通信的socket计数减1
    }

    close(listen_socket); // 父进程让监听socket计数减1,
                          // 此时会关掉监听socket(因为之前子进程已经有此操作)
    return 0;
}
