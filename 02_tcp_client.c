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
    socklen_t addrlen = sizeof(struct sockaddr_in);

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        ERRORLOG("fail to socket");
    }

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
    serveraddr.sin_port = htons(atoi(argv[2]));

   
    if(connect(sockfd, (struct sockaddr *)&serveraddr, addrlen) < 0)
    {
        ERRORLOG("fail to connect");
    }

    char msg[128] = "";
    while(1)
    {
        fgets(msg, sizeof(msg), stdin);
        msg[strlen(msg)-1] = 0;

        if(send(sockfd, msg, sizeof(msg), 0) < 0)
        {
            ERRORLOG("fail to send");
        }

        if(strncmp(msg, "quit", 4) == 0)
        {
            exit(0);
        }

        if(recv(sockfd, msg, sizeof(msg), 0) < 0)
        {
            ERRORLOG("fail to recv");
        }

        printf("from server: %s\n", msg);
    }

    close(sockfd);

    return 0;
}
