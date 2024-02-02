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
#include <ctype.h>
#include "chatRoom.h"
#include "threadpool.h"


#define SERVER_PORT 9999
#define SERVER_IP "127.0.0.1"
#define LISTEN_SIZE 128
#define BUFFER_SIZE 128


void Off(int arg)
{
    printf("服务端关闭。。。\n");
    exit(-1);
}

void * pthread_Fun(int *arg)
{
    pthread_detach(pthread_self()); 
    int acceptfd = *arg;
    char recvBuffer[BUFFER_SIZE];
    memset(recvBuffer, 0, sizeof(recvBuffer));

    int ret;

    char sendBuffer[BUFFER_SIZE];
    memset(sendBuffer, 0, sizeof(sendBuffer));

    while (1)
    {
        ret = recv(acceptfd, recvBuffer, sizeof(recvBuffer), 0);
        if (ret == -1)
        {
            perror("recv error");
            exit(-1);
        }
        printf("%s\n", recvBuffer);
        //scanf("%s", sendBuffer);
        
        ret = send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
        if (ret == -1)
        {
            perror("send error");
            exit(-1);
        }
        memset(recvBuffer, 0, sizeof(recvBuffer));
        memset(sendBuffer, 0, sizeof(sendBuffer));
    }

        pthread_exit(NULL);
}

//是否存在此好友
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

//自定义打印
int printStruct(void *arg)
{
    int ret = 0;
    chatRoomMessage* info = (chatRoomMessage*)arg;
    printf("accountNumber:%s\tname:%s\n", 
             info->accountNumber, info->name);
    return ret;
}

int main()
{
    /*将Ctrl+z设置为退出程序*/
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTSTP, Off);
    pthread_mutex_t * message_mutex;
    pthread_cond_t message_cond;
    /*初始化*/
    chatRoomMessage *Message = NULL;
    json_object *obj = NULL;
    Friend *Info = NULL;
    MYSQL *conn = NULL;
    friendNode *node = NULL;
    Friend *client = NULL;
    Friend * online = NULL;

    chatRoomInit(&Message, &obj, Info, client, online, conn, existenceOrNot, printStruct, node);

    threadpool_t *pool = NULL;
    int minThreads;
    int maxThreads;
    int queueCapacity;
    threadPoolInit(pool, minThreads, maxThreads, queueCapacity);    

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    int enableopt = 1;
    //设置端口复用
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enableopt, sizeof(enableopt));
    if (ret == -1)
    {
        perror("setsockopt error");
        exit(-1);
    }
    
    struct sockaddr_in clientAddress;
    memset(&clientAddress, 0, sizeof(clientAddress));
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = htons(SERVER_PORT);
    // clientAddress.sin_addr.s_addr = INADDR_ANY;
    //inet_pton(AF_INET, SERVER_IP, &clientAddress.sin_addr.s_addr);
    socklen_t clientAddressLen = sizeof(clientAddress);

    ret = bind(sockfd, (struct sockaddr*)&clientAddress, clientAddressLen);
    if (ret == -1)
    {
        perror("bind error");
        exit(-1);
    }
    
    ret = listen(sockfd, LISTEN_SIZE);
    if (ret == -1)
    {
        perror("listen error");
        exit(-1);
    }

    

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    
    // socklen_t clientAddressLen = sizeof(clientAddress);
    char recvBuffer[BUFFER_SIZE];
    memset(recvBuffer, 0, sizeof(recvBuffer));
    
    char sendBuffer[BUFFER_SIZE];
    memset(sendBuffer, 0, sizeof(sendBuffer));

    while (1)
    {
        int acceptfd = accept(sockfd, (struct sockaddr *)&clientAddress, &clientAddressLen);
        if (acceptfd == -1)
        {
            perror("accept error");
            exit(-1);
        }
        /*注册*/
        recv(acceptfd, recvBuffer, sizeof(recvBuffer), 0);
        if (!strncmp(recvBuffer, "1", sizeof(recvBuffer)))
        {
            pthread_mutex_lock(message_mutex);
            strncpy(sendBuffer, "请注册", sizeof(sendBuffer) - 1);

            memset(sendBuffer, 0, sizeof(sendBuffer));
            send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
            pthread_mutex_lock(message_mutex);

            if (!chatRoomInsert( sendBuffer, Message, obj, conn))
            {
                memset(recvBuffer, 0, sizeof(recvBuffer));
                strncpy(sendBuffer, "注册失败", sizeof(sendBuffer) - 1);
                send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                continue;
            }
            else
            {
                strncpy(sendBuffer, "注册成功", sizeof(sendBuffer) - 1);
                send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
            }
        }
        else if (strncmp(recvBuffer, "2",sizeof(recvBuffer)))
        {
            if (!chatRoomLogIn(Message, obj, client, conn))
            {
                memset(recvBuffer, 0, sizeof(recvBuffer));
                strncpy(sendBuffer, "登录失败", sizeof(sendBuffer) - 1);
                send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                continue;
            }
            else
            {
                strncpy(sendBuffer, "登录成功", sizeof(sendBuffer) - 1);
                send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
            }
        }
        
        
        threadPoolAddTask(pool, (void *)pthread_Fun, (void *) &acceptfd);
        // pthread_create(&tip, NULL, (void *)pthread_Fun, (void *)&acceptfd);
        
        

    }
    
    




    close(sockfd);


    return 0;
}
