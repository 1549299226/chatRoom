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
#include <sys/select.h>
#include "chatRoom.h"
#include "threadpool.h"
#include "hashtable.h"
#include <json-c/json_object.h>


#define SERVER_PORT 9999
#define MAX_LISTEN  128

#define BUFFER_SIZE     128

/* 用单进程/线程 实现并发 */

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
    /* 创建套接字 句柄 */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket error");
        exit(-1);
    }
    //printf("sockfd:%d\n", sockfd);

    /*初始化*/
    chatRoomMessage *Message = NULL;
    json_object *obj;
    Friend *Info = NULL;
    MYSQL *conn = NULL;
    friendNode *node = NULL;
    Friend *client = NULL;
    Friend * online = NULL;
    HashTable *onlineTable = NULL;

    chatRoomInit(&Message, &obj, &Info, &client, &online, &conn, existenceOrNot, printStruct, node, &onlineTable);

    threadpool_t *pool = NULL;
    int minThreads;
    int maxThreads;
    int queueCapacity;
    threadPoolInit(pool, minThreads, maxThreads, queueCapacity);

    int enableopt = 1;
    //设置端口复用
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enableopt, sizeof(enableopt));
    /* 将本地的IP和端口绑定 */
    struct sockaddr_in localAddress;
    bzero((void *)&localAddress, sizeof(localAddress));
    localAddress.sin_family = AF_INET;
    localAddress.sin_port = htons(SERVER_PORT);
    localAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    
    socklen_t localAddressLen = sizeof(localAddress);
    ret = bind(sockfd, (struct sockaddr *)&localAddress, localAddressLen);
    if (ret == -1)
    {
        perror("bind error");
        exit(-1);
    }

    /* 监听 */
    ret = listen(sockfd, MAX_LISTEN);
    if (ret == -1)
    {
        perror("listen error");
        exit(-1);
    }

    fd_set readSet;
    /* 清空集合 */
    FD_ZERO(&readSet);
    /* 把监听的文件描述符添加到读集合中, 让内核帮忙检测 */
    FD_SET(sockfd, &readSet);

    int maxfd = sockfd;

    #if 0
    /* 设置超时 */
    struct timeval timeValue;
    bzero(&timeValue, sizeof(timeValue));
    timeValue.tv_sec = 5;
    timeValue.tv_usec = 0;
    #endif

    fd_set tmpReadSet;
    /* 清除脏数据 */
    bzero(&tmpReadSet, sizeof(tmpReadSet));
    char recvBuffer[BUFFER_SIZE];
    memset(recvBuffer, 0, sizeof(recvBuffer));
    
    char sendBuffer[BUFFER_SIZE];
    memset(sendBuffer, 0, sizeof(sendBuffer));
    
    while (1)
    {
        /* 备份读集合 */
        tmpReadSet = readSet;
        ret = select(maxfd + 1, &tmpReadSet, NULL, NULL, NULL);
        if (ret == -1)
        {
            perror("select error");
            break;
        }

        /* 如果sockfd在tmpReadSet集合里面, 说明有新的连接请求到达 */
        if (FD_ISSET(sockfd, &tmpReadSet))
        {
            int acceptfd = accept(sockfd, NULL, NULL);
            if (acceptfd == -1)
            {
                perror("accpet error");
                break;
            }
            while (1)
            {
                                /*注册*/
                recv(acceptfd, recvBuffer, sizeof(recvBuffer), 0);
                if (!strncmp(recvBuffer, "1", sizeof(recvBuffer)))
                {
                    // pthread_mutex_lock(&message_mutex);
                    // pthread_cond_wait(&message_cond);
                    strncpy(sendBuffer, "请注册", sizeof(sendBuffer) - 1);
                    memset(sendBuffer, 0, sizeof(sendBuffer));
                    send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                    
                    memset(recvBuffer, 0, sizeof(recvBuffer));
                    recv(acceptfd, recvBuffer, sizeof(recvBuffer), 0);
                    printf("%s\n",recvBuffer);
                    printf("HERE----------\n");
                    chatRoomObjAnalyze(recvBuffer, Message, obj);
                    // pthread_cond_signal(&message_cond);
                    // pthread_mutex_lock(message_mutex);

                    if (!chatRoomInsert(Message, conn))
                    {
                        memset(recvBuffer, 0, sizeof(recvBuffer));
                        strncpy(sendBuffer, "账号错误", sizeof(sendBuffer) - 1);
                        send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                        continue;
                    }
                    else
                    {
                        strncpy(sendBuffer, "账号正确", sizeof(sendBuffer) - 1);
                        send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                        continue;
                    }
                }
                else if (!!strncmp(recvBuffer, "2",sizeof(recvBuffer)))     /*登录*/
                {
                    strncpy(sendBuffer, "请登录", sizeof(sendBuffer) - 1);
                    send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                    memset(sendBuffer, 0, sizeof(sendBuffer));

                    memset(recvBuffer, 0, sizeof(recvBuffer));    /*读取传来的信息*/
                    
                    recv(acceptfd, recvBuffer, sizeof(recvBuffer), 0);
                    printf("---recvBuffer:%s\n", recvBuffer);
                    chatRoomObjAnalyze(recvBuffer, Message, obj);
                    if (!chatRoomLogIn(acceptfd, Message, client, conn, onlineTable))
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
                        break;
                    }
                }
            
            /*询问好友是否在线 在线返回好友套接字fd */
            ret = serchFriendIfOnline(online, recvBuffer);
            if (ret > 0)   /*此时好友在线*/
            {
                memset(sendBuffer, 0, sizeof(sendBuffer));  /*清空缓存区*/
                strncpy(sendBuffer, "好友在线", sizeof(sendBuffer));
                send(sockfd, sendBuffer, sizeof(sendBuffer), 0);
                memset(sendBuffer, 0, sizeof(sendBuffer)); 
                
            }
            if (ret == 0)   /*此时没有好友 或者好友用户名不正确*/
            {
                memset(sendBuffer, 0, sizeof(sendBuffer));  /*清空缓存区*/
                strncpy(sendBuffer, "你没有好友 或者好友用户名不正确", sizeof(sendBuffer));
                send(sockfd, sendBuffer, sizeof(sendBuffer), 0);
                memset(sendBuffer, 0, sizeof(sendBuffer)); 
            }
            if (ret = -1)
            {
                memset(sendBuffer, 0, sizeof(sendBuffer));  /*清空缓存区*/
                strncpy(sendBuffer, "此时好友不在线", sizeof(sendBuffer));
                send(sockfd, sendBuffer, sizeof(sendBuffer), 0);
                memset(sendBuffer, 0, sizeof(sendBuffer)); 
            }
            }
            
            

            /* 将通信的句柄 放到读集合 */
            FD_SET(acceptfd, &readSet);

            /* 更新maxfd的值 */
            maxfd = maxfd < acceptfd ? acceptfd : maxfd;
        }
        
        /* 程序到这个地方: 说明可能有通信 */
        for (int idx = 0; idx <= maxfd; idx++)
        {
            if (idx != sockfd && FD_ISSET(idx, &tmpReadSet))
            {
                char buffer[BUFFER_SIZE];
                /* 清除脏数据 */
                bzero(buffer, sizeof(buffer));
                /* 程序到这里, 一定有通信(老客户) */
                int readBytes = read(idx, buffer, sizeof(buffer) - 1);
                if (readBytes < 0)
                {
                    perror("read error");
                    /* 将该通信句柄从监听的读集合中删掉 */
                    FD_CLR(idx, &readSet);
                    /* 关闭文件句柄 */
                    close(idx);
                    /* 这边要做成continue..., 让下一个已ready的fd句柄进行通信 */
                    continue;
                }
                else if (readBytes == 0)
                {
                    printf("客户端断开连接...\n");
                    /* 将该通信句柄从监听的读集合中删掉 */
                    FD_CLR(idx, &readSet);
                    /* 关闭通信句柄 */
                    close(idx);
                    /* 这边要做成continue..., 让下一个已ready的fd句柄进行通信 */
                    continue;
                }
                else
                {
                    printf("%s\n", buffer);
                    threadPoolAddTask(pool, (void *)pthread_Fun, (void *)&idx);
                    for (int jdx = 0; jdx < readBytes; jdx++)
                    {
                        buffer[jdx] = toupper(buffer[jdx]);
                    }

                    /* 发回客户端 */
                    write(idx, buffer, readBytes);
                    usleep(500);
                }
            }
        }

    
    }

    /* 关闭文件描述符 */
    close(sockfd);






    return 0;
}