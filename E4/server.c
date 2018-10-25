#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_MESSAGE_LEN 200

int main(int argc, char* argv[]) {
    if (argc != 2) {
        perror("wrong parameters!\n");
        return 0;
    }
    unsigned short port = atoi(argv[1]);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(-1);
    }

    struct sockaddr_in my_addr;
    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family      = AF_INET;
    my_addr.sin_port        = htons(port);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int isockoptval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &isockoptval, sizeof(isockoptval)) == -1) {
        perror("setsockopt fail\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    int ret = bind(sockfd, ( struct sockaddr* )&my_addr, sizeof(my_addr));
    if (ret != 0) {
        perror("binding");
        close(sockfd);
        exit(-1);
    }

    ret = listen(sockfd, 5);
    if (ret != 0) {
        perror("listen");
        close(sockfd);
        exit(-1);
    }

    fd_set         readfdset;
    struct timeval timeout;
    timeout.tv_sec  = 5;
    timeout.tv_usec = 5000000;
    while (1) {
        FD_ZERO(&readfdset);
        FD_SET(sockfd, &readfdset);
        FD_SET(0, &readfdset);
        ret = select(sockfd + 1, &readfdset, NULL, NULL, &timeout);
        if (ret < -1) {
            perror("select");
        }
        else {
            if (FD_ISSET(sockfd, &readfdset)) {
                char               cli_ip[INET_ADDRSTRLEN] = { 0 };
                struct sockaddr_in client_addr;
                socklen_t          cliaddr_len = sizeof(client_addr);

                // 取出客户端已完成的连接
                int connfd = accept(sockfd, ( struct sockaddr* )&client_addr, &cliaddr_len);
                if (connfd < 0) {
                    perror("accept");
                    close(sockfd);
                    exit(-1);
                }

                pid_t pid = fork();
                if (pid < 0) {
                    perror("fork");
                    _exit(-1);
                }
                else if (0 == pid) { //子进程 接收客户端的信息，并发还给客户端
                    close(sockfd);   // 关闭监听套接字

                    char recv_buf[MAX_MESSAGE_LEN] = { 0 };
                    int  recv_len                  = 0;

                    // 打印客户端的 ip 和端口
                    memset(cli_ip, 0, sizeof(cli_ip)); // 清空
                    inet_ntop(AF_INET, &client_addr.sin_addr, cli_ip, INET_ADDRSTRLEN);
                    printf("client ip=%s,port=%d\n", cli_ip, ntohs(client_addr.sin_port));

                    // 接收数据
                    while ((recv_len = recv(connfd, recv_buf, sizeof(recv_buf), 0)) > 0) {
                        if (strncmp(recv_buf, "exit", 4) == 0) {
                            break;
                        }
                        recv_buf[recv_len] = '\0';
                        printf("message from[%s:%d]>>> %s\n", cli_ip, client_addr.sin_port, recv_buf); // 打印数据
                        send(connfd, recv_buf, recv_len, 0);                                           // 给客户端回数据
                    }

                    printf("client_port %d closed!\n", ntohs(client_addr.sin_port));
                    close(connfd); //关闭已连接套接字
                    exit(0);
                }
                else if (pid > 0) { // 父进程
                    close(connfd);  //关闭已连接套接字
                }
            }
            if (FD_ISSET(0, &readfdset)) {
                char recv_buf[MAX_MESSAGE_LEN] = { 0 };
                int  recv_len                  = read(0, recv_buf, sizeof(recv_buf));
                if (strncmp(recv_buf, "exit", 4) == 0) {
                    close(sockfd);
                    exit(0);
                }
            }
        }
    }
    close(sockfd);
    return 0;
}
