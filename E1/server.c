#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("missing parameter\n");
        return 0;
    }
    printf("initialize server...\n");
    int                sock_fd;
    int                client_fd;
    int                maxfd;
    int                ret;
    fd_set             readfdset;
    struct timeval     timeout;
    struct sockaddr_in addr;
    struct sockaddr_in clientaddr;
    struct in_addr     inaddr;
    int                addrlen = sizeof(clientaddr);
    char               message[200];
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(atoi(argv[2]));
    ret             = inet_aton(argv[1], &inaddr);
    addr.sin_addr   = inaddr;

    sock_fd         = socket(AF_INET, SOCK_STREAM, 0);
    int isockoptval = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &isockoptval, sizeof(isockoptval)) == -1) {
        perror("setsockopt fail\n");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    ret = bind(sock_fd, ( struct sockaddr* )&addr, sizeof(addr));
    if (ret < 0) {
        perror("bind");
        exit(-1);
    }
    printf("server ready\n\n\n");
    ret             = listen(sock_fd, 5);
    timeout.tv_sec  = 5;
    timeout.tv_usec = 5000000;
    client_fd       = accept(sock_fd, ( struct sockaddr* )&clientaddr, &addrlen);
    bzero(message, sizeof(message));

    while (1) {
        FD_ZERO(&readfdset);
        FD_SET(client_fd, &readfdset);
        FD_SET(0, &readfdset);
        maxfd = (client_fd > sock_fd) ? client_fd : sock_fd;
        ret   = select(client_fd + 1, &readfdset, NULL, NULL, &timeout);
        if (ret < -1) {
            perror("select");
        }
        else {
            if (FD_ISSET(client_fd, &readfdset)) {
                bzero(message, sizeof(message));
                ret = recv(client_fd, message, sizeof(message), 0);
                if (ret == 0) {
                    printf("connect break\n");
                    break;
                }
                if (0 == strncmp(message, "exit", 4)) {
                    printf("receive exit message from client.\n");
                    break;
                }
                message[ret - 1] = '\0';
                printf(">>>%s\n", message);
                fflush(NULL);
            }
            if (FD_ISSET(0, &readfdset)) {
                ret = read(0, message, sizeof(message));
                if (ret < 0) {
                    perror("read");
                }
                else {
                    message[ret] = '\0';
                }
                send(client_fd, message, strlen(message), 0);
            }
        }
    }
    close(client_fd);
    close(sock_fd);
}
