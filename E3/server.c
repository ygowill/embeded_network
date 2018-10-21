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

#define MAX_CONN 10
#define MAX_MESSAGE_LEN 200

int fd_arr[MAX_CONN];

void initFdArr() {
    for (int i = 0; i < MAX_CONN; i++) {
        fd_arr[i] = -1;
    }
}

int addFD(int fd) {
    for (int i = 0; i < MAX_CONN; i++) {
        if (fd_arr[i] == -1) {
            fd_arr[i] = fd;
            return 0;
        }
    }
    return -1;
}

void clear_conn() {
    for (int i = 0; i < MAX_CONN; i++) {
        if (fd_arr[i] != -1) {
            close(fd_arr[i]);
        }
    }
}

void Server(char* ip, char* port) {
    int                sock_fd;
    int                maxfd = -1;
    int                ret;
    fd_set             readfdset;
    struct timeval     timeout;
    struct sockaddr_in addr;
    struct sockaddr_in clientaddr;
    struct in_addr     inaddr;
    int                addrlen = sizeof(clientaddr);
    addr.sin_family            = AF_INET;
    addr.sin_port              = htons(atoi(port));
    ret                        = inet_aton(ip, &inaddr);
    addr.sin_addr              = inaddr;

    initFdArr();
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    addFD(sock_fd);
    addFD(0);

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

    ret = listen(sock_fd, 5);

    timeout.tv_sec  = 60;
    timeout.tv_usec = 60000000;
    printf("server ready\n\n\n");

    while (1) {
        FD_ZERO(&readfdset);
        for (int i = 0; i < MAX_CONN; ++i) {
            if (fd_arr[i] != -1) {
                FD_SET(fd_arr[i], &readfdset);
                if (fd_arr[i] > maxfd) {
                    maxfd = fd_arr[i];
                }
            }
        }

        ret = select(maxfd + 1, &readfdset, NULL, NULL, &timeout);
        if (ret == -1) {
            perror("select");
        }
        else if (ret == 0) {
            printf("select timeout......");
            break;
        }
        else {
            for (int i = 0; i < MAX_CONN; i++) {
                if (FD_ISSET(0, &readfdset)) {
                    char buf[MAX_MESSAGE_LEN];
                    bzero(buf, sizeof(buf));
                    ret = read(0, buf, sizeof(buf));
                    if (ret < 0) {
                        perror("read");
                    }
                    if (strncmp(buf, "exit", 4) == 0)
                        exit(0);
                }
                if (i == 0 && fd_arr[i] != -1 && FD_ISSET(fd_arr[i], &readfdset)) {
                    socklen_t len    = sizeof(clientaddr);
                    int       new_fd = accept(sock_fd, ( struct sockaddr* )&clientaddr, &len);
                    if (-1 != new_fd) {
                        printf("get a new link from [%s](fd:%d)\n", inet_ntoa(clientaddr.sin_addr), new_fd);
                        if (-1 == addFD(new_fd)) {
                            perror("fd_arr is full,close new_fd\n");
                            close(new_fd);
                        }
                    }
                    continue;
                }
                if (fd_arr[i] != -1 && FD_ISSET(fd_arr[i], &readfdset)) {
                    char buf[MAX_MESSAGE_LEN];
                    bzero(buf, sizeof(buf));
                    ssize_t size  = recv(fd_arr[i], buf, sizeof(buf), 0);
                    buf[size - 1] = '\0';
                    if (size == 0 || size == -1) {
                        printf("remote client(fd:%d) close\n", fd_arr[i]);
                        for (int j = 0; j < MAX_CONN; ++j) {
                            if (fd_arr[j] == fd_arr[i]) {
                                fd_arr[j] = -1;
                                break;
                            }
                        }
                        close(fd_arr[i]);
                        FD_CLR(fd_arr[i], &readfdset);
                    }
                    else {
                        if (strncmp(buf, "exit", 4) == 0) {
                            close(fd_arr[i]);
                            FD_CLR(fd_arr[i], &readfdset);
                            printf("remote client(fd:%d) close\n", fd_arr[i]);
                            fd_arr[i] = -1;
                            break;
                        }
                        send(fd_arr[i], buf, sizeof(buf), 0);
                        printf("fd:%d,msg:%s\n", fd_arr[i], buf);
                    }
                }
            }
        }
    }

    clear_conn();
    close(sock_fd);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("missing parameter\n");
        return 0;
    }
    printf("initialize server...\n");
    Server(argv[1], argv[2]);
    return 0;
}
