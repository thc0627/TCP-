#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

#define ERRORLOG(errorlog) do{\
                                printf("%s--%s--%d\n", __FILE__, __func__, __LINE__);\
                                perror(errorlog);\
                                exit(1);\
                            }while(0)

typedef struct
{
    struct sockaddr_in clientaddr;
    int fd_client;
    char msg[128];
}MSG;

void handler(int arg)
{
    wait(NULL);
}

void* pthread_fun(void *arg)
{
    char filename[64] = "";
    char filepath[64] = "";
    ssize_t bytes = 0;
    MSG msg1 = *(MSG *)arg;
    int filefd = 0;
    char text[1024] = "";
    char head[]="HTTP/1.1 200 OK\r\n"\
                "Content-Type: text/html\r\n"\
                "\r\n";
    char err[]="HTTP/1.1 404 Not Found\r\n"\
               "Content-Type: text/html\r\n"\
               "\r\n"\
               "<HTML><BODY>File not found</BODY></HTML>";
    
    printf("[%s--%d]\n", inet_ntoa(msg1.clientaddr.sin_addr), ntohs(msg1.clientaddr.sin_port));

    printf("from client: %s\n", msg1.msg);
    sscanf(msg1.msg, "%*[^/]/%s %*s", filename);
    if(strncmp(filename, "HTTP/1.1", strlen("HTTP/1.1")) == 0)
    {
        strcpy(filename, "about.html");
    }
    sprintf(filepath, "/home/thc/work/network/sqlite/%s", filename);

    if((filefd = open(filepath, O_RDONLY)) < 0)
    {
        ERRORLOG("fail to open");
        
        if(send(msg1.fd_client, err, sizeof(err), 0) < 0)
        {
            ERRORLOG("fail to send");
        }

        exit(1);
    }
    else if (filefd > 0)
    {
        if(send(msg1.fd_client, head, sizeof(head), 0) < 0)
        {
            ERRORLOG("fail to send");
        }
    }

    while((bytes = read(filefd, text, sizeof(text))) > 0)
    {
        if(send(msg1.fd_client, text, bytes, 0) < 0)
        {
            ERRORLOG("fail to send");
        }
    }

    close(filefd);
    close(msg1.fd_client);

    exit(1);
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
    int fd = 0;
    ssize_t byte = 0;
    char msg1[128] = "";
    MSG msg;

    signal(SIGCHLD, handler);

    while(1)
    {
        if((fd_client = accept(sockfd, (struct sockaddr *)&clientaddr, &addrlen)) < 0)
        {
            ERRORLOG("fail to accept");
        }

        msg.clientaddr = clientaddr;
        msg.fd_client = fd_client;

        if((fd = fork()) < 0)
        {
            ERRORLOG("fail to fork");
        }

        if(fd > 0)
        {

        }
        else if (fd == 0)
        {
            if((byte = recv(fd_client, msg1, sizeof(msg1), 0)) < 0)
            {
                ERRORLOG("fail to recv");
            }
            else if (byte == 0)
            {
                printf("the client quited\n");
                close(fd_client);
                exit(1);
            }

            strcpy(msg.msg, msg1);

            pthread_t thread;
            if(pthread_create(&thread, NULL, pthread_fun, &msg) != 0)
            {
                ERRORLOG("fail to pthread_creat");
            }
            pthread_detach(thread);
        }
    }
    
    close(sockfd);

    return 0;
}
