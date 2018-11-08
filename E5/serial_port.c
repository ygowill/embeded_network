#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

//定义波特率
#define BAUDRATE B115200

#define FALSE 0
#define TRUE 1

volatile int STOP = FALSE;

int main(int argc, char** argv) {
    if (argc != 2) {
        perror("wrong parameters");
        exit(1);
    }
    char* const    DEVICE = argv[1];
    int            fd, c, res;
    struct termios oldtio, newtio;
    char           buf[255];
    fd_set         readfdset;
    struct timeval timeout;
    int            ret;

    fd = open(DEVICE, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror("打开串口失败");
        exit(-1);
    }

    tcgetattr(fd, &oldtio);
    bzero(&newtio, sizeof(newtio));

    newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR | ICRNL;
    newtio.c_oflag = 0;
    newtio.c_lflag = ICANON;

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);

    printf("start.\n\n");

    while (STOP == FALSE) {
        FD_ZERO(&readfdset);
        FD_SET(fd, &readfdset);
        FD_SET(0, &readfdset);
        ret = select(fd + 1, &readfdset, NULL, NULL, &timeout);
        if (ret < -1) {
            perror("select");
        }
        else {
            if (FD_ISSET(fd, &readfdset)) {
                bzero(buf, sizeof(buf));
                ret = read(fd, buf, sizeof(buf));
                if (ret == 0) {
                    printf("connect break\n");
                    break;
                }
                if (0 == strncmp(buf, "exit", 4)) {
                    printf("receive exit message.\n");
                    STOP = TRUE;
                    continue;
                }
                buf[ret - 1] = '\0';
                printf(">>>%s\n", buf);
                fflush(NULL);
            }
            if (FD_ISSET(0, &readfdset)) {
                ret = read(0, buf, sizeof(buf));
                if (ret < 0) {
                    perror("read");
                }
                else {
                    buf[ret] = '\0';
                }
                write(fd, buf, strlen(buf));
            }
        }
    }

    tcsetattr(fd, TCSANOW, &oldtio);
    close(fd);
    return 0;
}
