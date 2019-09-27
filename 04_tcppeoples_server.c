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

typedef struct
{
    struct sockaddr_in clientaddr;
    int fd_client;
    int *flag;
}MSG;


void* receive_fun(void *arg)
{
    int fd_client = *(int *)arg;
    char msg[128] = "";
    ssize_t byte = 0;
    MSG msg1 = *(MSG *)arg;
    
    printf("[%s--%d]\n", inet_ntoa(msg1.clientaddr.sin_addr), ntohs(msg1.clientaddr.sin_port));

    while(1)
    {
        if((byte = recv(msg1.fd_client, msg, sizeof(msg), 0)) < 0)
        {
            ERRORLOG("fail to recv");
        }
        else if (byte == 0)
        {
            printf("the client quited\n");
            (*(msg1.flag))--;
            if(*(msg1.flag) == 0)
            {
                exit(0);
            }
            close(msg1.fd_client);
            pthread_exit(NULL);
        }

        if(strncmp(msg, "quit", 4) == 0)
        {
            (*(msg1.flag))--;
            if(*(msg1.flag) == 0)
            {
                exit(0);
            }
            close(msg1.fd_client);
            pthread_exit(NULL);
        }

        printf("[%s--%d]:%s\n", inet_ntoa(msg1.clientaddr.sin_addr), ntohs(msg1.clientaddr.sin_port), msg);
        strcat(msg, " ^_^");
        if(send(msg1.fd_client, msg, sizeof(msg), 0) < 0)
        {
            ERRORLOG("fail to send");
        }
    }

    close(fd_client);
}

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
    MSG msg1;
    int flag = 0;
    while(1)
    {
        if((fd_client = accept(sockfd, (struct sockaddr *)&clientaddr, &addrlen)) < 0)
        {
            ERRORLOG("fail to accept");
        }

        flag++;
        msg1.clientaddr = clientaddr;
        msg1.fd_client = fd_client;
        msg1.flag = &flag;

        pthread_t thread;
        if(pthread_create(&thread, NULL, receive_fun, &msg1) != 0)
        {
            ERRORLOG("fail to pthread_creat");
        }
        pthread_detach(thread);
    }
    
    close(sockfd);

    return 0;
}
