#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<netdb.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<errno.h>

#define PORT 23333

int main(int argc,char *argv[])
{
    int sockfd;
    char sendbuffer[200];
    char recvbuffer[200];
    struct sockaddr_in server_addr;
    if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        fprintf(stderr,"Socket Error:%s\a\n",strerror(errno));
        exit(1);
    }
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(PORT);
    server_addr.sin_addr.s_addr=htonl("127.0.0.1");
    if(connect(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1){
        fprintf(stderr,"Connect error:%s\n",strerror(errno));
        exit(1);
    }
    while(1)
    {
        printf("Please input your word:\n");
        scanf("%s",sendbuffer);
        printf("\n");
        if(strcmp(sendbuffer,"quit")==0)
            break;
        send(sockfd,sendbuffer,sizeof(sendbuffer),0);
        recv(sockfd,recvbuffer,200,0);
        printf("recv data :%s\n",recvbuffer);
    }
    close(sockfd);
    exit(0);
}