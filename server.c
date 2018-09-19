#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdlib.h>

#define PORT 23333
#define BACKLOG 5
#define MAXDATASIZE 1024

typedef struct _CLIENT
{
  int fd;
  char name[100];
  struct sockaddr_in addr;
  char data[MAXDATASIZE];
} CLIENT;

void main(int argc ,char **argv){
    printf("start initialization...\n");
    int conn_num, max_conn, maxfd, sockfd;
    int nready;
    fd_set rset, allset;
    int listenfd, connectfd;
    struct sockaddr_in server;
    CLIENT client[FD_SETSIZE];
    char recv_buf[MAXDATASIZE];
    char send_buf[MAXDATASIZE];
    int sin_size;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("Creating socket failed.\n");
        exit(1);
    }

    int opt = SO_REUSEADDR;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listenfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1){
        perror("Bind error.\n");
        exit(1);
    }

    if (listen(listenfd, BACKLOG) == -1){
        perror("listen() error\n");
        exit(1);
    }

    //初始化select   
    maxfd = listenfd;
    max_conn = -1;
    for (conn_num = 0; conn_num < FD_SETSIZE; conn_num++){
        client[conn_num].fd = -1;
    }
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);  //将监听socket加入select检测的描述符集合
    FD_SET(STDIN_FILENO, &allset);  //将标准输入加入select检测的描述符集合

    printf("server ready.\n");
    int flag=1;
    while (flag){
        struct sockaddr_in addr;
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);    //调用select   
        printf("Select() break and the return num is %d. \n", nready);

        if (FD_ISSET(listenfd, &rset)){
            printf("Accept a connection.\n");
            //调用accept，返回服务器与客户端连接的socket描述符   
            sin_size = sizeof(struct sockaddr_in);
            if ((connectfd = accept(listenfd, (struct sockaddr *)&addr, (socklen_t *) & sin_size)) == -1){
                perror("Accept() error\n");
                continue;
            }

            //将新客户端的加入数组   
            for (conn_num = 0; conn_num < FD_SETSIZE; conn_num++){
                if (client[conn_num].fd < 0){
                    char buffer[20];
                    client[conn_num].fd = connectfd;   //保存客户端描述符
                    memset(buffer, '0', sizeof(buffer));
                    sprintf(buffer, "Client[%.2d]", conn_num);
                    memcpy(client[conn_num].name, buffer, strlen(buffer));
                    client[conn_num].addr = addr;
                    memset(buffer, '0', sizeof(buffer));
                    sprintf(buffer, "Only For Test!");
                    memcpy(client[conn_num].data, buffer, strlen(buffer));
                    printf("Got a connection from %s:%d.\n", inet_ntoa(client[conn_num].addr.sin_addr),ntohs(client[conn_num].addr.sin_port));
                    printf("Add a new connection:%s\n",client[conn_num].name);
                    break;
                }
            }

            if (conn_num == FD_SETSIZE)
                printf("Too many clients\n");
            FD_SET(connectfd, &allset);

            maxfd = connectfd > maxfd ? connectfd : maxfd;
            max_conn = conn_num > max_conn ? conn_num : max_conn;

            if (--nready <= 0)
                continue;
        }

        if(FD_ISSET(STDIN_FILENO, &rset)){
            fgets(send_buf,MAXDATASIZE,stdin);
            send_buf[strlen(send_buf)-1]='\0';
            sendto(listenfd,send_buf,strlen(send_buf),0,
                   (struct sockaddr *)&server,sizeof(server));
        }

        for (conn_num = 0; conn_num <= max_conn; conn_num++){
            if ((sockfd = client[conn_num].fd) < 0)
                continue;
            if (FD_ISSET(sockfd, &rset)){
                printf("Receive from connect fd[%d].\n", conn_num);
                if(!recv(sockfd, recv_buf, MAXDATASIZE, 0)){
                    close(sockfd);  //关闭socket连接   
                    printf("%s closed. User's data: %s\n", client[conn_num].name, client[conn_num].data);
                    FD_CLR(sockfd, &allset);
                    client[conn_num].fd = -1;
                }
                else{
                    if(strcmp(recv_buf,"stop server")==0){
                        flag = 0;
                        break;
                    }
                    printf("client( %s ) >>> %s\n", &client[conn_num], recv_buf);
                }
                if (--nready <= 0)
                    break;
            }
        }
    }
    close(listenfd);
}