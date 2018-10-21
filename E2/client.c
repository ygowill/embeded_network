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
    printf("initialize client...\n");
    int                sock_fd;
    int                client_fd;
    int                ret;
    struct timeval     timeout;
    fd_set             readfdset;
    struct sockaddr_in addr;
    struct sockaddr_in clientaddr;
    struct in_addr     inaddr;
    socklen_t          addrlen = sizeof(struct sockaddr);
    char               message[200];
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(atoi(argv[2]));
    ret             = inet_aton(argv[1], &inaddr);
    addr.sin_addr   = inaddr;

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

    bzero(message, sizeof(message));
    printf("client ready\n\n\n");
    while (1) {
        timeout.tv_sec  = 5;
        timeout.tv_usec = 5000000;
        FD_ZERO(&readfdset);
        FD_SET(sock_fd, &readfdset);
        FD_SET(0, &readfdset);
        ret = select(sock_fd + 1, &readfdset, NULL, NULL, &timeout);
        if (ret < 0) {
            perror("select");
        }
        else {
            if (FD_ISSET(0, &readfdset)) {
                bzero(message, sizeof(message));
                ret = read(0, message, sizeof(message));
                if (ret < 0) {
                    perror("read");
                }
                if (strncmp(message, "shutdown", 8) == 0) {
                    close(sock_fd);
                    return 0;
                }
                sendto(sock_fd, message, sizeof(message), 0, ( struct sockaddr* )&addr, sizeof(addr));
            }
            if (FD_ISSET(sock_fd, &readfdset)) {
                bzero(message, sizeof(message));
                ret = recvfrom(sock_fd, message, sizeof(message), 0, ( struct sockaddr* )&addr, &addrlen);
                if (0 == ret) {
                    printf("server not in work\n");
                    break;
                }
                else {
                    message[ret - 1] = '\0';
                    printf(">>>%s\n", message);
                    fflush(NULL);
                }
            }
        }
    }
    close(sock_fd);
}
