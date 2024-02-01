#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <error.h>

#define BUFFER_SIZE 128
#define SERVER_PORT 10000
#define SERVER_IP "172.27.132.115"

void * pthread_Fun(int *arg)
{ 
    int sockfd = * arg;
    char recvBuffer[BUFFER_SIZE];
    memset(recvBuffer, 0, sizeof(recvBuffer));

    char sendBuffer[BUFFER_SIZE];
    memset(sendBuffer, 0, sizeof(sendBuffer));  
    
    struct sockaddr_in recvAddress;
    memset(&recvAddress, 0, sizeof(recvAddress));
    socklen_t recvAddressLen = sizeof(recvAddress);

    recvAddress.sin_family = AF_INET;
    recvAddress.sin_port = htons(SERVER_PORT);

    inet_pton(AF_INET, SERVER_IP, &recvAddress.sin_addr.s_addr); 

    int ret = 0;
    ret = connect(sockfd, (struct sockaddr *)&recvAddress, recvAddressLen); 
        if (ret == -1)
        {
            perror("connect error");
            exit(-1);
        } 
    while (1)
    {   
        scanf("%s", sendBuffer);
        send(sockfd, sendBuffer, sizeof(sendBuffer), 0);

        recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
        printf("%s\n", recvBuffer);
        memset(recvBuffer, 0, sizeof(recvBuffer));
        memset(sendBuffer, 0, sizeof(sendBuffer));
    }
}

void Off(int arg)
{
    printf("客户端关闭。。。\n");
    exit(-1);
}

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTSTP, Off);

    if (sockfd == -1)
    {
        perror("sockfd error");
        exit(-1);
    }
    pthread_t tip;
    pthread_create(&tip, NULL, (void *)pthread_Fun, (void *)&sockfd);

    

    while (1)
    {
        sleep(3);
    }
    
    
    close(sockfd);
    return 0;
}
