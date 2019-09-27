#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#define ERRORLOG(errorlog) do{\
                                printf("%s--%s--%d\n", __FILE__, __func__, __LINE__);\
                                perror(errorlog);\
                                exit(1);\
                            }while(0)

int main(int argc, char const *argv[])
{
    int sockfd = 0;
    struct sockaddr_in serveraddr;
    struct sockaddr_in clientaddr;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        ERRORLOG("fail to socket");
    }

    int on = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) < 0)
    {
        ERRORLOG("fail to setsockopt");
    }

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
    serveraddr.sin_port = htons(atoi(argv[2]));

    if(bind(sockfd, (struct sockaddr *)&serveraddr, addrlen) < 0)
    {
        ERRORLOG("fail to bind");
    }

    if(listen(sockfd, 5) < 0)
    {
        ERRORLOG("fail to listen");
    }

    int fd_client = 0;
    if((fd_client = accept(sockfd, (struct sockaddr *)&clientaddr, &addrlen)) < 0)
    {
        ERRORLOG("fail to accept");
    }
    printf("[%s--%d]\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

    char msg[128] = "";
    ssize_t byte = 0;
    int num = 0;
    while(1)
    {
        if((byte = recv(fd_client, msg, sizeof(msg), 0)) < 0)
        {
            ERRORLOG("fail to recv");
        }
        else if (byte == 0)
        {
            printf("the client quited");
        }

        if(strncmp(msg, "quit", 4) == 0)
        {
            exit(0);
        }

        printf("from client: %s\n", msg);
        strcat(msg, " ^_^");
        if(send(fd_client, msg, sizeof(msg), 0) < 0)
        {
            ERRORLOG("fail to send");
        }
    }

    close(sockfd);
    close(fd_client);
    return 0;
}
