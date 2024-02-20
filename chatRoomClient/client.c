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
#include <json-c/json_object.h>
#include <json-c/json.h>

#define BUFFER_SIZE 128
#define SERVER_PORT 9999
#define SERVER_IP "127.0.0.1"

#define ACCOUNTNUMBER 6
#define NAMESIZE 12


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
    chatContent * friendMessage = NULL;
    chatRoomInit(&Message, &friendMessage, &obj, Info, client, online, &conn, existenceOrNot, printStruct, node); /*初始化*/

     
    
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
        while (1)
        {       
            fristInterface();
            scanf("%s", flag);
            
            if (!strncmp(flag, "1", sizeof(flag)))
            {
                
                send(sockfd, flag, sizeof(flag), 0);    //写入选项

                recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
                printf("%s\n", recvBuffer);
                usleep(500);
                memset(recvBuffer, 0, sizeof(recvBuffer));

                chatRoomClientMeassage(userBuf, Message, obj);    /*将信息写入到userBuf中*/
                send(sockfd, userBuf, sizeof(userBuf), 0);          /*将信息写入通信句柄中*/

                recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);    /*读取是否通信成功*/
                if (strncmp(recvBuffer, "注册成功", sizeof(recvBuffer) -1))  //注册失败进入if语句
                {
                    memset(Message->accountNumber, 0, sizeof(Message->accountNumber));
                    memset(Message->name, 0, sizeof(Message->name));
                    memset(Message->password, 0, sizeof(Message->password));
                    memset(Message->mail, 0, sizeof(Message->mail));
                    printf("注册失败\n");
                    continue;
                }
                else
                {
                    printf("注册成功\n");
                    loginInterface();
                    memset(flag, 0, sizeof(flag));

                    continue;
                }
            }
            else if(!strncmp(flag, "2", sizeof(flag)))
            {   
                system("clear");
                send(sockfd, flag, sizeof(flag), 0);   //写入选项      
                recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);  //读取返回的
                printf("%s\n", recvBuffer);
                usleep(500);
                memset(recvBuffer, 0, sizeof(recvBuffer));
                memset(userBuf, 0, sizeof(userBuf));

                chatRoomClientLogIn(userBuf,Message, obj);
                send(sockfd, userBuf, sizeof(userBuf), 0);

                recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
                if (!strncmp(recvBuffer, "登录成功", sizeof(recvBuffer) -1))
                {
                    
                    printf("登录成功\n");
                    memset(flag, 0, sizeof(flag));
                    enterInterface();
                    sleep(2);
                    break;
                }
                else
                {
                    printf("登录失败\n");
                    memset(Message->accountNumber, 0, sizeof(Message->accountNumber));
                    memset(Message->name, 0, sizeof(Message->name));
                    memset(Message->password, 0, sizeof(Message->password));
                    memset(Message->mail, 0, sizeof(Message->mail));
                    memset(flag, 0, sizeof(flag));
                    sleep(2);
                    continue;
                }
            }
            else
            {
                printf("输入有误，请重新选择\n");
                continue;
           
            }
        }
    
        //主界面
        while (1)
        {
            mainInterface();
            scanf("%s", flag);
            send(sockfd, flag, sizeof(flag), 0);
            //添加好友
            if (!strncmp(flag, "1", sizeof(flag)))
            {      
                chatRoomMessage *friendMessage = (chatRoomMessage *)malloc(sizeof(chatRoomMessage));
                memset(friendMessage, 0, sizeof(friendMessage)); 
                
                friendMessage->accountNumber = (char *)malloc(sizeof(ACCOUNTNUMBER));
                memset(friendMessage->accountNumber, 0, sizeof(friendMessage->accountNumber));

                friendMessage->name = (char *)malloc(sizeof(NAMESIZE));
                memset(friendMessage->name, 0, sizeof(friendMessage->name));

                memset(flag, 0, sizeof(flag));
                recv(sockfd, recvBuffer,sizeof(recvBuffer), 0);
                printf("%s\n", recvBuffer);
                memset(recvBuffer, 0, sizeof(recvBuffer));
                while (1)
                {   
                    scanf("%s", flag);
                    send(sockfd, flag, sizeof(flag), 0);
                    if (!strncmp(flag, "1", sizeof(flag)))
                    {
                        recv(sockfd, recvBuffer,sizeof(recvBuffer), 0);
                        printf("%s\n", recvBuffer);
                        memset(recvBuffer, 0, sizeof(recvBuffer));
                        
                        scanf("%s", friendMessage->accountNumber);
                        send(sockfd, friendMessage->accountNumber, sizeof(friendMessage->accountNumber), 0);
                        memset(friendMessage->accountNumber, 0, sizeof(friendMessage->accountNumber));
                        
                        recv(sockfd, recvBuffer,sizeof(recvBuffer), 0);
                        printf("查询到的名称:%s\n", recvBuffer);
                        if (!strncmp(recvBuffer, "添加失败，查询此人失败,请重新查询", sizeof(recvBuffer)))
                        {
                            printf("%s\n", recvBuffer);
                            memset(flag, 0, sizeof(flag));
                            continue;
                        }
                        else
                        {
                            printf("是否要添加此人为好友:\n1.是   2.否\n");
                            scanf("%s", flag);
                            send(sockfd, flag, sizeof(flag), 0);
                            if (!strncmp(flag, "1", sizeof(flag)))
                            {
                                recv(sockfd, recvBuffer,sizeof(recvBuffer), 0);

                                if (!strncmp(recvBuffer, "添加失败，插入此人失败，请重新查询", sizeof(recvBuffer)))
                                {
                                    printf("%s\n", recvBuffer);
                                    memset(flag, 0, sizeof(flag));
                                    continue;
                                }
                                else if (!strncmp(recvBuffer, "添加好友成功", sizeof(recvBuffer)))
                                {
                                    printf("%s\n", recvBuffer);
                                    break;
                                }
                            }
                            else if (!strncmp(flag, "2", sizeof(flag)))
                            {
                                recv(sockfd, recvBuffer,sizeof(recvBuffer), 0);
                                printf("%s\n", recvBuffer);
                                memset(recvBuffer, 0, sizeof(recvBuffer));
                                continue;
 
                            }
                            else
                            {
                                printf("输入内容不符\n");
                                continue;
                            }
                        }
                    }
                    else if (!strncmp(flag, "2", sizeof(flag)))     //用昵称查找
                    {
                        recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
                        printf("%s\n", recvBuffer);
                        memset(recvBuffer, 0, sizeof(recvBuffer));

                        scanf("%s", friendMessage->name);
                        send(sockfd, friendMessage->name, sizeof(friendMessage->name), 0);
                        memset(friendMessage->name, 0, sizeof(friendMessage->name));
                        
                        recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
                        printf("查询到的账号:%s\n", recvBuffer);
                        memset(recvBuffer, 0, sizeof(recvBuffer));
                        
                        if (!strncmp(recvBuffer, "添加失败，查询此人失败, 请重新查询", sizeof(recvBuffer)))
                        {
                            printf("%s\n", recvBuffer);
                            memset(flag, 0, sizeof(flag));
                            continue;
                        }
                        else
                        {
                            printf("是否要添加此人为好友:\n1.是   2.否\n");
                            scanf("%s", flag);
                            send(sockfd, flag, sizeof(flag), 0);
                            if (!strncmp(flag, "1", sizeof(flag)))
                            {
                                recv(sockfd, recvBuffer,sizeof(recvBuffer), 0);

                                if (!strncmp(recvBuffer, "添加失败，插入此人失败，请重新查询", sizeof(recvBuffer)))
                                {
                                    printf("%s\n", recvBuffer);
                                    memset(flag, 0, sizeof(flag));
                                    continue;
                                }
                                else if (!strncmp(recvBuffer, "添加好友成功", sizeof(recvBuffer)))
                                {
                                    printf("%s\n", recvBuffer);
                                    break;
                                }
                            }
                            else if (!strncmp(flag, "2", sizeof(flag)))
                            {
                                memset(recvBuffer, 0, sizeof(recvBuffer));
                                recv(sockfd, recvBuffer,sizeof(recvBuffer), 0);
                                printf("%s\n", recvBuffer);
                                memset(recvBuffer, 0, sizeof(recvBuffer));
                                continue;
 
                            }
                            else
                            {
                                printf("输入内容不符\n");
                                continue;
                            }
                        }
                    }
                    else
                    {
                        printf("输入有误，请重新输入\n");
                        continue;
                    }
                
                
                }
                
                
                


                continue;

                
                
            }
            
            /*聊天功能*/
            else if (!strncmp(flag, "2", sizeof(flag)))
            {
                memset(flag, 0, sizeof(flag));

                recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
                printf("%s\n", recvBuffer);
                //查看好友 
                memset(recvBuffer, 0, sizeof(recvBuffer));
                //send(sockfd, flag, sizeof(flag), 0);

                
                scanf("%s", flag);
                send(sockfd, flag, sizeof(flag), 0);
                if (!strncmp(flag, "1", sizeof(flag)))
                {
                    memset(flag, 0, sizeof(flag));

                    /*群聊 to do..*/
                }
                /*私聊*/
                else if (!strncmp(flag, "2", sizeof(flag)))
                {
                    memset(flag, 0, sizeof(flag));

                    /*先清零*/
                        
                    printf("以下是所有好友的信息:\n");
                    //这个应该写在服务器上在服务其中查询好友的列表
                    while (1)
                    {
                        printf("1、输入私聊对象的名字进行聊天\n");
                        printf("2、退出返回上一界面\n");
                        scanf("%s", flag);

                        if (!strncmp(flag, "1", sizeof(flag)))
                        {
                            printf("请输入要聊天的好友姓名\n");
                            scanf("%s", friendMessage->friendName);
                            /*先清零缓冲区*/
                            memset(sendBuffer, 0, sizeof(sendBuffer));
                            if (friendIsExit(client, friendMessage, friendMessage->friendName) == 1) /*好友存在时*/
                            {
                                /*发送好友的名字到服务端*/
                                strncpy(sendBuffer, friendMessage->friendName, sizeof(sendBuffer));
                                ret = send(sockfd, sendBuffer, sizeof(flag), 0);
                                if (ret < 0)      /*发送失败*/
                                {
                                    perror("send Is the friend online error");
                                    memset(sendBuffer, 0, sizeof(sendBuffer));
                                    continue;
                                }
                                /*清空缓冲区*/
                                memset(recvBuffer, 0, sizeof(recvBuffer));
                                /*接收好友是否在线的信息*/
                                ret = recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
                                if (ret == -1) 
                                {
                                        perror("recv error");  // 打印错误信息
                                        printf("接收错误，返回上一级\n");
                                        continue;
                                        // 处理接收错误的情况
                                } 
                                else if (ret == 0) 
                                {
                                    printf("Connection closed by peer\n");  // 连接被关闭
                                        // 处理连接关闭的情况
                                    printf("连接关闭错误，返回上一级\n");
                                    continue;
                                } 
                                else 
                                {
                                        // 接收到数据成功
                                    //recvBuffer[ret] = '\0';  // 在接收到的数据末尾添加字符串结束符
                                    if (!strncmp(recvBuffer, "好友在线", sizeof(recvBuffer)))
                                    {
                                        printf("好友在线\n");
                                        char * chatMsg = NULL;
                                        /*发送消息  to do ..*/
                                        chatRoomPrivateChat(chatMsg, sockfd);
                                    }
                                    // if (!strncmp(recvBuffer, "你没有好友 或者好友用户名不正确", sizeof(recvBuffer)))
                                    // {
                                    //     printf("你没有好友 或者好友用户名不正确, 返回上一级\n");
                                    //     continue;
                                    // }
                                    if (!strncmp(recvBuffer, "此时好友不在线", sizeof(recvBuffer)))
                                    {
                                        printf("此时好友不在线, 返回上一级\n");
                                        continue;
                                    }
                                
                                }
                            }

                        }
                        else if (!strncmp(flag, "2", sizeof(flag)))
                        {
                            mainInterface();
                            break;
                        }
                        else 
                        {
                            printf("无效的输入，请重新输入\n");
                            mainInterface();
                            continue;
                        }

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
        
        
        
        while (1)
        {
            threadPoolAddTask(pool, (void *)pthread_Fun, (void *) &sockfd);
        }
        
        
    
    }
    close(sockfd);
    return 0;
}
