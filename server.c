#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>
#define RET_VAL(VAL,FUNC_NAME) do{\
                    if(VAL < 0) \
                    {\
                        perror(#FUNC_NAME);\
                        exit(-1);\
                    }\
                }\
                while(0)
int main(int argc, char * argv[]) {
    int sock_fd;
    int client_fd;
    int maxfd;
    int ret;
    fd_set readfdset;
    struct timeval timeout;
    struct sockaddr_in addr;
    struct sockaddr_in clientaddr;
    struct in_addr inaddr;
    int addrlen = sizeof(clientaddr);
    char message[200];
    char *sendstr = "hello server\n";
    addr.sin_family = AF_INET;
    addr.sin_port = htons(3456);
    ret = inet_aton("127.0.0.1", &inaddr);
    RET_VAL(ret, inet_aton);
    addr.sin_addr = inaddr;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    ret = bind(sock_fd, (struct sockaddr *) &addr, sizeof(addr));
    if (ret < 0) {
        perror("bind");
        exit(-1);
    }

    ret = listen(sock_fd, 5);
    RET_VAL(ret, listen);
    timeout.tv_sec = 5;
    timeout.tv_usec = 5000;
    client_fd = accept(sock_fd, (struct sockaddr *) &clientaddr, &addrlen);
    bzero(message, sizeof(message));

    while (1) {
        timeout.tv_sec = 5;
        timeout.tv_usec = 5000;
        FD_ZERO(&readfdset);
        FD_SET(client_fd, &readfdset);
        FD_SET(0, &readfdset);
        maxfd = (client_fd > sock_fd) ? client_fd : sock_fd;
        ret = select(client_fd + 1, &readfdset, NULL, NULL, &timeout);
        if (ret < -1) {
            perror("select");
        } else if (0 == ret) {
            printf("time is over\n");
        } else {
            if (FD_ISSET(client_fd, &readfdset)) {
                bzero(message, sizeof(message));
                ret = recv(client_fd, message, sizeof(message), 0);
                RET_VAL(ret, recv);
                if (0 == strncmp(message, "close", 5)) {
                    break;
                }
                if (ret == 0) {
                    printf("connect break\n");
                    break;
                }
                printf("%s", message);
            }
            if (FD_ISSET(0, &readfdset)) {
                ret = read(0, message, sizeof(message));
                if (ret < 0) {
                    perror("read");
                } else {
                    message[ret] = '\0';
                }
                printf("%s\n", message);
                send(client_fd, message, strlen(message), 0);
            }
        }
        printf("timtout.tv_sec=%d\n", timeout.tv_sec);

    }
    printf("while exit\n");
    ret = recv(client_fd, message, sizeof(message), 0);
    RET_VAL(ret, recv);
    printf("%s", message);
    ret = send(client_fd, sendstr, strlen(sendstr), 0);
    RET_VAL(ret, send);
    printf("send string success\n");
    close(client_fd);
    close(sock_fd);
}