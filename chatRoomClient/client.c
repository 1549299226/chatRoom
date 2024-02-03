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
#include "threadpool.h"
#include "chatRoom.h"

#define BUFFER_SIZE 128
#define SERVER_PORT 9999
#define SERVER_IP "172.18.101.255"

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

//自定义打印
int printStruct(void *arg)
{
    int ret = 0;
    chatRoomMessage* info = (chatRoomMessage*)arg;
    printf("accountNumber:%s\tname:%s\n", 
             info->accountNumber, info->name);
    return ret;
}

int existenceOrNot(void *arg1, void *arg2)
{
    chatRoomMessage *idx1 = (chatRoomMessage *) arg1;
    chatRoomMessage *idx2 = (chatRoomMessage *) arg2;
    // char * idx1 = (char *)arg1;
    // char * idx2 = (char *)arg2;
    int result = 0;
    result = strcmp(idx1->name, idx2->name);

    return result;
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
    
    chatRoomMessage *Message = NULL;
    json_object *obj = NULL;
    Friend *Info = NULL;
    MYSQL *conn = NULL;
    friendNode *node = NULL;
    Friend *client = NULL;
    Friend * online = NULL;

    chatRoomInit(&Message, &obj, Info, client, online, conn, existenceOrNot, printStruct, node); /*初始化*/

     
    
    struct sockaddr_in recvAddress;
    memset(&recvAddress, 0, sizeof(recvAddress));
    socklen_t recvAddressLen = sizeof(recvAddress);

    threadpool_t *pool = NULL;
    int minThreads;
    int maxThreads;
    int queueCapacity;
    threadPoolInit(pool, minThreads, maxThreads, queueCapacity);  /*初始化线程池*/

    recvAddress.sin_family = AF_INET;
    recvAddress.sin_port = htons(SERVER_PORT);
    ret = inet_pton(AF_INET, SERVER_IP, &recvAddress.sin_addr.s_addr);
    if (ret == -1)
    {
        perror("inet_pton error");
        exit(-1);
    }
    ret = connect(sockfd, (struct sockaddr *)&recvAddress, recvAddressLen);   /*建立通信*/
    if (ret == -1)
    {
        perror("connect error");
        exit(-1);
    }
    

    char recvBuffer[BUFFER_SIZE];
    memset(recvBuffer, 0, sizeof(recvBuffer));

    char sendBuffer[BUFFER_SIZE];
    memset(sendBuffer, 0, sizeof(sendBuffer)); 
    welcomeInterface();

    char userBuf[BUFFER_SIZE];
    memset(userBuf, 0, sizeof(userBuf));

    char * flag = (char *)malloc(sizeof(char));
    memset(flag, 0, sizeof(flag));

    while (1)
    {   

        scanf("%s", flag);
        
        if (!strncmp(flag, "1", sizeof(flag)))
        {
            
            send(sockfd, flag, sizeof(flag), 0);
            loginInterface();
            recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
            printf("%s\n", recvBuffer);
            memset(recvBuffer, 0, sizeof(recvBuffer));

            chatRoomClientMeassage(userBuf, Message, obj);    /*将信息写入到userBuf中*/
            send(sockfd, userBuf, sizeof(userBuf), 0);          /*将信息写入通信句柄中*/

            recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);    /*读取是否通信成功*/
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
        else if(!strncmp(flag, "2", sizeof(flag)))
        {
            send(sockfd, flag, sizeof(flag), 0);
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
        else if (!strncmp(flag, "3", sizeof(flag)))
        {
            printf("请选择1、群聊 2、私聊 \n");
            scanf("%s", flag);
            if (!strncmp(flag, "1", sizeof(flag)))
            {

            }
            else if (!strncmp(flag, "2", sizeof(flag)))
            {
                printf("以下是所有好友的信息:\n");
                balanceBinarySearchTreeInOrderTravel(Info);

                printf("1、输入私聊对象的名字进行聊天\n");
                printf("2、退出返回上一界面\n");
                scanf("%s", flag);

                if (!strncmp(flag, "1", sizeof(flag)))
                {
                    
                }

            }
        }
        else
        {
            printf("输入有误，请重新选择\n");
                continue;

        }

        threadPoolAddTask(pool, (void *)pthread_Fun, (void *) &sockfd);
    }
    
    
    close(sockfd);
    return 0;
}
