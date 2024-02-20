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

#define ACCOUNTNUMBER 6
#define NAMESIZE 12
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
    chatContent * friendMessage = NULL;
    
    chatRoomInit(&Message, &friendMessage, &obj, &Info, &client, &online, &conn, existenceOrNot, printStruct, node, &onlineTable);

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
                    send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                    memset(sendBuffer, 0, sizeof(sendBuffer));

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
                else if (!strncmp(recvBuffer, "2",sizeof(recvBuffer)))     /*登录*/
                {
                    strncpy(sendBuffer, "请登录", sizeof(sendBuffer) - 1);
                    send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                    memset(sendBuffer, 0, sizeof(sendBuffer));

                    memset(recvBuffer, 0, sizeof(recvBuffer));    /*读取传来的信息*/
                    
                    recv(acceptfd, recvBuffer, sizeof(recvBuffer), 0);
                    chatRoomObjAnalyze(recvBuffer, Message, obj);
                    if (!chatRoomLogIn(acceptfd, Message, client, conn, onlineTable))
                    {

                        memset(recvBuffer, 0, sizeof(recvBuffer));
                        strncpy(sendBuffer, "登录成功", sizeof(sendBuffer) - 1);
                        send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                        memset(sendBuffer, 0, sizeof(sendBuffer));
                        
                        break;
                    }
                    else
                    {
                        strncpy(sendBuffer, "登录失败", sizeof(sendBuffer) - 1);
                        send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                        continue;
                    }
                }
            }

            /*主界面*/
            while (1)
            {
                //查看选项
                recv(acceptfd, recvBuffer, sizeof(recvBuffer), 0);
                if (!strncmp(recvBuffer, "1", sizeof(recvBuffer)))
                {
                    //添加好友
                    strncpy(sendBuffer, "请选择 1.用账号查找 2.昵称查找", sizeof(sendBuffer) - 1);
                    send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);

                    chatRoomMessage *friendMessage = (chatRoomMessage *)malloc(sizeof(chatRoomMessage));
                    memset(friendMessage, 0, sizeof(friendMessage));
                    
                    friendMessage->accountNumber = (char *)malloc(sizeof(ACCOUNTNUMBER));
                    memset(friendMessage->accountNumber, 0, sizeof(friendMessage->accountNumber));

                    friendMessage->name = (char *)malloc(sizeof(NAMESIZE));
                    memset(friendMessage->name, 0, sizeof(friendMessage->name));

                    char * accountNumber = (char *)malloc(ACCOUNTNUMBER);
                    memset(accountNumber, 0, ACCOUNTNUMBER);

                    char * name = (char *)malloc(NAMESIZE);
                    memset(name, 0, NAMESIZE);
                    char buffer[BUFFER_SIZE];
                    memset(buffer, 0, sizeof(buffer));

                    memset(sendBuffer, 0, sizeof(sendBuffer));

                    while (1)
                    {
                        

                        recv(acceptfd, recvBuffer,sizeof(recvBuffer), 0);
                        

                        if (!strncmp(recvBuffer, "1", sizeof(recvBuffer)))      //用账号查找  
                        {
                            printf("请输入账号\n");
                            strncpy(sendBuffer, "请输入账号", sizeof(sendBuffer));    
                            send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                            memset(sendBuffer, 0, sizeof(sendBuffer));
                            memset(recvBuffer, 0, sizeof(recvBuffer));
                            
                            recv(acceptfd, recvBuffer,sizeof(recvBuffer), 0);
                            friendMessage->accountNumber = recvBuffer;
                            snprintf(buffer, sizeof(buffer), "SELECT accountNumber , name FROM chatRoom WHERE accountNumber = '%s'", friendMessage->accountNumber);
                            if (mysql_query(conn, buffer))
                            {
                                strncpy(sendBuffer, "添加失败，查询此人失败,请重新查询", sizeof(sendBuffer));    
                                send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                                memset(sendBuffer, 0, sizeof(sendBuffer));
                                memset(friendMessage->accountNumber, 0, sizeof(friendMessage->accountNumber));
                                memset(friendMessage->name, 0, sizeof(friendMessage->name));
                                continue;
                            }
                            else        /*需要加一个将查询出的结果放到数组中，再放入好友数据库中*/
                            {
                                
                                        /*又可以改进的地方  可以改为查到将用户的信息打印出来，之后再确定要不要加此人为好友，
                                                                是，则向该用户发送一个信息是否接受该用户的好友申请，
                                                                选1接受 2不接受应该如此，现在完成的是点击加好友就直接加到了自己的好友表中是不太友好的*/
                                MYSQL_RES *res = mysql_use_result(conn);
                                if (res != NULL) 
                                {
                                    MYSQL_ROW row;
                                    if ((row = mysql_fetch_row(res)) != NULL) 
                                    {
                                        snprintf(accountNumber, ACCOUNTNUMBER + 1, "%s", row[0]);
                                        snprintf(name, NAMESIZE + 1, "%s", row[1]);

                                            // 处理完一行数据后的其他操作
                                    }
                                    mysql_free_result(res);  // 释放查询结果集
                                }
                                printf("账号---%s\n", accountNumber);
                                
                                
                             
                                send(acceptfd, name, NAMESIZE + 1, 0);
                                memset(recvBuffer, 0, sizeof(recvBuffer));
                                recv(acceptfd, recvBuffer,sizeof(recvBuffer), 0);
                                if (!strncmp(recvBuffer, "1", sizeof(recvBuffer)))
                                {
                           
                                    snprintf(buffer, sizeof(buffer), "INSERT INTO Friend%s (accountNumber , name) VALUES ('%s', '%s')", Message->accountNumber, accountNumber, name);
                                    
                                    if (mysql_query(conn, buffer))
                                    {
                                        strncpy(sendBuffer, "添加失败，插入此人失败，请重新查询", sizeof(sendBuffer));    
                                        send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                                        memset(sendBuffer, 0, sizeof(sendBuffer));
                                        memset(friendMessage->accountNumber, 0, sizeof(friendMessage->accountNumber));
                                        memset(friendMessage->name, 0, sizeof(friendMessage->name));
                                        memset(name, 0, NAMESIZE + 1);
                                        memset(accountNumber, 0, ACCOUNTNUMBER + 1);

                                        continue;
                                    }
                                    friendMessage->accountNumber = accountNumber;
                                    friendMessage->name = name;
                                    //插入到好友列表
                                    balanceBinarySearchTreeInsert(client, friendMessage);
                                    strncpy(sendBuffer, "添加好友成功", sizeof(sendBuffer));    
                                    send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                                    memset(sendBuffer, 0, sizeof(sendBuffer));
                                    memset(friendMessage->accountNumber, 0, sizeof(friendMessage->accountNumber));
                                    memset(friendMessage->name, 0, sizeof(friendMessage->name));
                                    memset(name, 0, NAMESIZE + 1);
                                    memset(accountNumber, 0, ACCOUNTNUMBER + 1);
                                    printf("添加成功");
                                    sleep(1);
                                    break;
                                }    
                                else if (!strncmp(recvBuffer, "2", sizeof(recvBuffer)))
                                {
                                    strncpy(sendBuffer, "返回成功", sizeof(sendBuffer));    
                                    send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                                    memset(sendBuffer, 0, sizeof(sendBuffer));
                                    memset(friendMessage->accountNumber, 0, sizeof(friendMessage->accountNumber));
                                    memset(friendMessage->name, 0, sizeof(friendMessage->name));
                                    continue;
                                }
                                else
                                {
                                    memset(friendMessage->accountNumber, 0, sizeof(friendMessage->accountNumber));
                                    memset(friendMessage->name, 0, sizeof(friendMessage->name));
                                    continue;
                                }
                            }
                        }
                        else if (!strncmp(recvBuffer, "2", sizeof(recvBuffer)))     //用昵称查找
                        {
                            printf("请输入昵称\n");
                            strncpy(sendBuffer, "请输入昵称", sizeof(sendBuffer));    
                            send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                            memset(sendBuffer, 0, sizeof(sendBuffer));

                            memset(recvBuffer, 0, sizeof(recvBuffer));
                            recv(acceptfd, recvBuffer,sizeof(recvBuffer), 0);
                            printf("name:%s\n", recvBuffer);

                            char buffer[BUFFER_SIZE];
                            memset(buffer, 0, sizeof(buffer));
                            friendMessage->name = recvBuffer;
                            snprintf(buffer, sizeof(buffer), "SELECT accountNumber , name FROM chatRoom WHERE name = '%s'", friendMessage->name);
                            if (mysql_query(conn, buffer))
                            {
                                strncpy(sendBuffer, "添加失败，查询此人失败,请重新查询", sizeof(sendBuffer));    
                                send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                                memset(sendBuffer, 0, sizeof(sendBuffer));
                                
                                continue;
                            }
                            else
                            {
                                MYSQL_RES *res = mysql_use_result(conn);
                                if (res != NULL) 
                                {
                                    MYSQL_ROW row;
                                    if ((row = mysql_fetch_row(res)) != NULL) 
                                    {
                                        snprintf(accountNumber, ACCOUNTNUMBER + 1, "%s", row[0]);
                                        snprintf(name, NAMESIZE + 1, "%s", row[1]);

                                            // 处理完一行数据后的其他操作
                                    }
                                    mysql_free_result(res);  // 释放查询结果集
                                }
                                printf("424----%s,%s\n", accountNumber, name);
                                send(acceptfd, accountNumber, ACCOUNTNUMBER + 1, 0);

                                                  /*这里少东西还，*/
                                
                                memset(recvBuffer, 0, sizeof(recvBuffer));
                                recv(acceptfd, recvBuffer,sizeof(recvBuffer), 0);
                                printf("431----%s\n", recvBuffer);
                                if (!strncmp(recvBuffer, "1", sizeof(recvBuffer)))
                                {
                                    //创建好友表   有问题   好友表没有标记出来
                                    //插入到好友列表
                                    printf("436----%s,%s\n", accountNumber, name);
                                     
                                    snprintf(buffer, sizeof(buffer), "INSERT INTO Friend%s(accountNumber ,name) VALUES ('%s', '%s')", Message->accountNumber, accountNumber, name);
                                    if (mysql_query(conn, buffer))
                                    {
                                        strncpy(sendBuffer, "添加失败，插入此人失败，请重新查询", sizeof(sendBuffer));    
                                        send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                                        memset(sendBuffer, 0, sizeof(sendBuffer));
                                        memset(friendMessage->accountNumber, 0, sizeof(friendMessage->accountNumber));
                                        memset(friendMessage->name, 0, sizeof(friendMessage->name));
                                        memset(name, 0, NAMESIZE + 1);
                                        memset(accountNumber, 0, ACCOUNTNUMBER + 1);
                                        continue;
                                    }

                                    //添加好友到树中
                                    friendMessage->accountNumber = accountNumber;
                                    friendMessage->name = name;
                                    balanceBinarySearchTreeInsert(client, friendMessage);
                                    strncpy(sendBuffer, "添加好友成功", sizeof(sendBuffer));    
                                    send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                                    memset(sendBuffer, 0, sizeof(sendBuffer));
                                    memset(friendMessage->accountNumber, 0, sizeof(friendMessage->accountNumber));
                                    memset(friendMessage->name, 0, sizeof(friendMessage->name));
                                    memset(name, 0, NAMESIZE + 1);
                                    memset(accountNumber, 0, ACCOUNTNUMBER + 1);
                                    printf("添加成功");
                                    sleep(1);
                                    break;
                                }    
                                else if (!strncmp(recvBuffer, "2", sizeof(recvBuffer)))
                                {
                                    strncpy(sendBuffer, "返回成功", sizeof(sendBuffer));    
                                    send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                                    memset(sendBuffer, 0, sizeof(sendBuffer));
                                    memset(friendMessage->accountNumber, 0, sizeof(friendMessage->accountNumber));
                                    memset(friendMessage->name, 0, sizeof(friendMessage->name));

                                    continue;
                                }
                                else
                                {
                                    memset(friendMessage->accountNumber, 0, sizeof(friendMessage->accountNumber));
                                    memset(friendMessage->name, 0, sizeof(friendMessage->name));
                                    continue;
                                }
                            }
                        }
                        else
                        {
                            continue;
                        }
                    }
                }
                else if (!strncmp(recvBuffer, "2", sizeof(recvBuffer)))
                {
                    strncpy(sendBuffer, "请选择1、群聊 2、私聊 ", sizeof(sendBuffer));
                    send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                    memset(sendBuffer, 0, sizeof(sendBuffer));

                    //选择聊天方式    
                    memset(recvBuffer, 0, sizeof(recvBuffer));
                    recv(acceptfd, recvBuffer, sizeof(recvBuffer), 0);
                    
                    if (!strncmp(recvBuffer, "1", sizeof(recvBuffer)))
                    {
                        memset(recvBuffer, 0, sizeof(recvBuffer));
                        
                        /*群聊 to do..*/
                    }
                    else if (!strncmp(recvBuffer, "2", sizeof(recvBuffer)))
                    {
                        memset(recvBuffer, 0, sizeof(recvBuffer));
                        balanceBinarySearchTreeInOrderTravel(client); //查询好友信息
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
                        if (ret == -1)
                        {
                            memset(sendBuffer, 0, sizeof(sendBuffer));  /*清空缓存区*/
                            strncpy(sendBuffer, "此时好友不在线", sizeof(sendBuffer));
                            send(sockfd, sendBuffer, sizeof(sendBuffer), 0);
                            memset(sendBuffer, 0, sizeof(sendBuffer)); 
                        }
                    }
                }   
                else if (!strncmp(recvBuffer, "3", sizeof(recvBuffer)))
                {
                    //删除好友
                }
                else if (!strncmp(recvBuffer, "0", sizeof(recvBuffer)))
                {
                    //退出登录
                }
                else if (!strncmp(recvBuffer, "X", sizeof(recvBuffer)))
                {
                    //注销登录
                }
                else
                {
                    printf("输入有误，请重新选择\n");
                    continue;

                    

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