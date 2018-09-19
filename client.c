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
    int ret;
    struct timeval timeout;
    fd_set readfdset;
    struct sockaddr_in addr;
    struct sockaddr_in clientaddr;
    struct in_addr inaddr;
    int addrlen = sizeof(clientaddr);
    char message[200];
    char *sendstr = "hello client\n";
    addr.sin_family = AF_INET;
    addr.sin_port = htons(3456);
    ret = inet_aton("127.0.0.1", &inaddr);
    RET_VAL(ret, inet_aton);
    addr.sin_addr = inaddr;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    ret = connect(sock_fd, (struct sockaddr *) &addr, sizeof(addr));
    RET_VAL(ret, connect);
    bzero(message, sizeof(message));
//    ret = recv(client_fd,message,sizeof(message),0);
//    RET_VAL(ret,recv);
//    printf("%s",message);
    while (1) {
        timeout.tv_sec = 5;
        timeout.tv_usec = 5000000;
        FD_ZERO(&readfdset);
        FD_SET(sock_fd, &readfdset);
        FD_SET(0, &readfdset);
        ret = select(sock_fd + 1, &readfdset, NULL, NULL, &timeout);
        if (ret < 0) {
            perror("select");
        } else if (0 == ret) {
            printf("time is over\n");
        } else {
            if (FD_ISSET(0, &readfdset)) {
                printf("select ok\n");
                ret = read(0, message, sizeof(message));
                if (ret < 0) {
                    perror("read");
                }
                send(sock_fd, message, strlen(message), 0);
                if (0 == strncmp(message, "close", 5)) {
                    break;
                }
            }
            if (FD_ISSET(sock_fd, &readfdset)) {
                ret = recv(sock_fd, message, sizeof(message), 0);
                if (0 == ret) {
                    printf("server not in work\n");
                    break;
                } else {
                    message[ret] = '\0';
                    printf("recv:%s\n", message);
                }
            }
        }
    }
    ret = send(sock_fd, sendstr, strlen(sendstr), 0);
    RET_VAL(ret, send);
    printf("send string success\n");

//    ret = recv(sock_fd,message,sizeof(message),0);
//    printf("recv from ser:%s",message);
    close(sock_fd);
}