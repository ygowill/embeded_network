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

int main(int argc, char * argv[]) {
    if(argc<3){
        printf("missing parameter\n");
        return 0;
    }
    printf("initialize client...\n");
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
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    ret = inet_aton(argv[1], &inaddr);
    addr.sin_addr = inaddr;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    ret = connect(sock_fd, (struct sockaddr *) &addr, sizeof(addr));
    bzero(message, sizeof(message));
    printf("client ready\n\n\n");
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
            //printf("time is over\n");
        } else {
            if (FD_ISSET(0, &readfdset)) {
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
                bzero(message, sizeof(message));
                ret = recv(sock_fd, message, sizeof(message), 0);
                if (0 == ret) {
                    printf("server not in work\n");
                    break;
                } else {
                    message[ret-1] = '\0';
                    printf(">>>%s\n", message);
                }
            }
        }
    }
    close(sock_fd);
}
