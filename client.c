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
#include "chatRoomInter.h"

#define BUFFER_SIZE 128
#define SERVER_PORT 10000
#define SERVER_IP "172.27.132.115"

void * pthread_Fun(int *arg)
{ 
    int sockfd = * arg;
    char sendBuffer[BUFFER_SIZE];
    memset(sendBuffer, 0, sizeof(sendBuffer));  
    
    struct sockaddr_in recvAddress;
    
    char recvBuffer[BUFFER_SIZE];
    memset(recvBuffer, 0, sizeof(recvBuffer));
     

    
    while (1)
    {   
        

    
        
        //scanf("%s", sendBuffer);
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
    int ret = 0;
    if (sockfd == -1)
    {
        perror("sockfd error");
        exit(-1);
    }
    

    char recvBuffer[BUFFER_SIZE];
    memset(recvBuffer, 0, sizeof(recvBuffer));

    char sendBuffer[BUFFER_SIZE];
    memset(sendBuffer, 0, sizeof(sendBuffer));  
    
    struct sockaddr_in recvAddress;
    memset(&recvAddress, 0, sizeof(recvAddress));
    socklen_t recvAddressLen = sizeof(recvAddress);

    char flag;
    
    recvAddress.sin_family = AF_INET;
    recvAddress.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &recvAddress.sin_addr.s_addr);

    ret = connect(sockfd, (struct sockaddr *)&recvAddress, recvAddressLen); 
    if (ret == -1)
    {
        perror("connect error");
        exit(-1);
    }
    
    while (1)
    {   
        welcomeInterface();

        scanf("%s", &flag);
        send(sockfd, &flag, sizeof(flag), 0);
        if (flag == '1')
        {
            loginInterface();
            recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
            if (strncmp(recvBuffer, "注册成功", sizeof(recvBuffer) -1))
            {
                continue;
            }
            else
            {
                printf("注册成功\n");
                fristInterface();
            }
        }
        else if(flag == '2')
        {

            recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
            if (strncmp(recvBuffer, "登录成功", sizeof(recvBuffer) -1))
            {
                continue;
            }
            else
            {
                printf("登录成功\n");
                enterInterface();
            }
        }
        
        
        
        pthread_t tip;
        pthread_create(&tip, NULL, (void *)pthread_Fun, (void *)&sockfd);
    }
    
    
    close(sockfd);
    return 0;
}
