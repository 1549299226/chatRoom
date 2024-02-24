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
#include <fcntl.h>

#define SERVER_PORT 9999
#define MAX_LISTEN  128

#define BUFFER_SIZE     1024
#define BUFFER      1024
#define ACCOUNTNUMBER 6
#define NAMESIZE 12
/* 用单进程/线程 实现并发 */
pthread_mutex_t mutex_server = PTHREAD_MUTEX_INITIALIZER;
void Off(int arg)
{
    printf("服务端关闭。。。\n");
    
    exit(-1);
}

void * pthread_Fun(int *arg)
{
    // pthread_detach(pthread_self()); 
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
int printStruct(void *arg1, void *arg2)
{
    int ret = 0;
    chatRoomMessage* info = (chatRoomMessage*)arg1;
    char * buffer = (char *)arg2;
    printf("90 -- %s\n", info->name);
    
    json_object * obj = NULL;
    
    printStructObj(buffer, info, obj);
    
    printf("94 -- %s\n", buffer);
    return ret;
}

void * handle_group_chat(void * arg)
{

}

/*要传进哈希表和他的结构体*/
void* handleClient(void* arg) 
{
    // pthread_detach(pthread_self());
    fdHash * hashHandle = (fdHash *)arg;  // 获取acceptfd
    pthread_t tid_groupchat;
    int acceptfd = hashHandle->sockfd;
    chatRoomMessage *Message = NULL;
    json_object *obj;
    Friend *Info = NULL;
    MYSQL *conn = NULL;
    friendNode *node = NULL;
    Friend *client = NULL;
    Friend * online = NULL;
    chatContent * friendMessage = NULL;
    chatHash * onlineHash = (chatHash *)malloc(sizeof(chatHash));
    groupChat * groupChatInfo = NULL;
    onlineHash->hashName = (char *)malloc(NAMESIZE); 
    onlineHash->sockfd = 0;
    chatRoomInit(&Message, &groupChatInfo,&friendMessage, &obj, &Info, &client, &online, &conn, existenceOrNot, compareFunc1, printStruct, node, &hashHandle->onlineTable);

    printf("127---fd:%d", acceptfd);
    char recvBuffer[BUFFER_SIZE];
    memset(recvBuffer, 0, sizeof(recvBuffer));
    
    char sendBuffer[BUFFER_SIZE];
    memset(sendBuffer, 0, sizeof(sendBuffer));
    // 这里放入原先的代码
    while (1) 
    {            
        while (1)
        {
            /*注册*/
            memset(recvBuffer, 0, sizeof(recvBuffer));
            int ret = recv(acceptfd, recvBuffer, sizeof(recvBuffer), 0);
            if (ret == 0)
            {
                printf("客户端%d关闭\n", acceptfd);
                /*调用退出登录代码 to do*/
                close(acceptfd);
                return NULL;
            }
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
                
                /*读出账号密码*/
                recv(acceptfd, recvBuffer, sizeof(recvBuffer), 0);
                chatRoomObjAnalyze(recvBuffer, Message, obj);

                char buffer[BUFFER_SIZE];
                memset(buffer, 0, sizeof(buffer));
                snprintf(buffer, sizeof(buffer), "SELECT name FROM chatRoom WHERE accountNumber = '%s'", Message->accountNumber);
                if (mysql_query(conn, buffer))
                {
                    printf("查无此人\n");    
                    exit(-1);
                }
                else        /*需要加一个将查询出的结果放到数组中，再放入好友数据库中*/
                {
                    MYSQL_RES *res = mysql_use_result(conn);
                    memset(buffer, 0, sizeof(buffer));
                    if (res != NULL) 
                    {
                        MYSQL_ROW row;
                        if ((row = mysql_fetch_row(res)) != NULL) 
                        {
                            //snprintf(Friend.accountNumber, sizeof(Friend.accountNumber), "%s", row[0]);
                            snprintf(buffer, sizeof(buffer), "%s", row[0]);

                        }
                        mysql_free_result(res);  // 释放查询结果集
                    }
                }
                /*本人的哈希 */
                onlineHash->hashName = buffer; 
                strncpy(onlineHash->hashName, buffer, ACCOUNTNUMBER);
                onlineHash->sockfd = acceptfd;

                // send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                // memset(sendBuffer, 0, sizeof(sendBuffer));

                // send(acceptfd, buffer, sizeof(buffer), 0);
                // printf("214----%s\n", buffer);
                
                
                // recv(acceptfd,recvBuffer, sizeof(recvBuffer), 0);
                //printf("218----%s\n", recvBuffer);
                //chatHashObjAnalyze(recvBuffer, onlineHash, obj);
                memset(recvBuffer, 0, sizeof(recvBuffer));    /*读取传来的信息*/
                
                if (!chatRoomLogIn(acceptfd, Message, client, conn, hashHandle->onlineTable, onlineHash))
                {
                    
                    
                    strncpy(sendBuffer, "登录成功", sizeof(sendBuffer) - 1);
                    send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                    
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
            printf("进入主界面\n");
            memset(recvBuffer, 0, sizeof(recvBuffer));
            int ret = recv(acceptfd, recvBuffer, sizeof(recvBuffer), 0);
            if (ret == 0)
            {
                printf("客户端%d关闭\n", acceptfd);
                /*调用退出登录代码 to do*/
                close(acceptfd);
                return NULL;

            }
            /*添加好友*/
            if (!strncmp(recvBuffer, "1", sizeof(recvBuffer)))
            {
                //添加好友
                strncpy(sendBuffer, "请选择 1.用账号查找 2.昵称查找3、返回上一级", sizeof(sendBuffer) - 1);
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
                    

                    ret = recv(acceptfd, recvBuffer,sizeof(recvBuffer), 0);
                    if (ret == 0)
                    {
                        printf("客户端%d关闭\n", acceptfd);
                        /*调用退出登录代码 to do*/
                        close(acceptfd);
                        return NULL;
                    }

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
                        if (mysql_query(conn, buffer) < 0)
                        {
                            strncpy(sendBuffer, "添加失败，查询此人失败,请重新查询", sizeof(sendBuffer));    
                            send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                            memset(sendBuffer, 0, sizeof(sendBuffer));
                            memset(friendMessage->accountNumber, 0, sizeof(friendMessage->accountNumber));
                            memset(friendMessage->name, 0, sizeof(friendMessage->name));
                            continue;
                        }
                        else if (!mysql_query(conn, buffer))
                        {
                            strncpy(sendBuffer, "没有此人，请重新添加", sizeof(sendBuffer));    
                            send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                            memset(sendBuffer, 0, sizeof(sendBuffer));
                            
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
                                printf("添加成功\n");
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
                        if (mysql_query(conn, buffer) < 0)
                        {
                            strncpy(sendBuffer, "添加失败，查询此人失败,请重新查询", sizeof(sendBuffer));    
                            send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                            memset(sendBuffer, 0, sizeof(sendBuffer));
                            
                            continue;
                        }
                        else if (!mysql_query(conn, buffer))
                        {
                            strncpy(sendBuffer, "没有此人，请重新添加", sizeof(sendBuffer));    
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
                            printf("399----%s,%s\n", accountNumber, name);
                            send(acceptfd, accountNumber, ACCOUNTNUMBER + 1, 0);

                                                /*这里少东西还，*/
                            
                            memset(recvBuffer, 0, sizeof(recvBuffer));
                            recv(acceptfd, recvBuffer,sizeof(recvBuffer), 0);
                            printf("406----%s\n", recvBuffer);
                            if (!strncmp(recvBuffer, "1", sizeof(recvBuffer)))
                            {
                                //创建好友表   有问题   好友表没有标记出来
                                //插入到好友列表
                                printf("411----%s,%s\n", accountNumber, name);
                                    
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
                                printf("添加成功\n");
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
                    /*返回上一级*/
                    else if (!strncmp(recvBuffer, "3", sizeof(recvBuffer))) 
                    {
                        break;
                    }  
                    else
                    {
                        printf("无效输入,请重新输入\n");
                        continue;
                    }
                }
            }

            else if (!strncmp(recvBuffer, "2", sizeof(recvBuffer)))//聊天功能
            {
                memset(recvBuffer, 0, sizeof(recvBuffer));
                while (1)
                {
                    //聊天功能
                    memset(sendBuffer, 0, sizeof(sendBuffer));
                    strncpy(sendBuffer, "请选择1、群聊 2、私聊 3、返回上一级(主界面)", sizeof(sendBuffer));
                    send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                    memset(sendBuffer, 0, sizeof(sendBuffer));

                    //选择聊天方式    
                    memset(recvBuffer, 0, sizeof(recvBuffer));
                    /*读取聊天方式选择结果*/
                    ret = recv(acceptfd, recvBuffer, sizeof(recvBuffer), 0);
                    if (ret == 0)
                    {
                        printf("客户端%d关闭\n", acceptfd);
                        /*调用退出登录代码 to do*/
                        close(acceptfd);
                        return NULL;
                    }
                    
                    /*群聊*/
                    if (!strncmp(recvBuffer, "1", sizeof(recvBuffer)))//群聊
                    {
                        while (1)
                        {
                            memset(sendBuffer, 0, sizeof(sendBuffer));
                            char  str_groupNameTra[BUFFER_SIZE];
                            ret = travelGroupChatName(conn, Message, str_groupNameTra);
                            /*查询出错*/
                            if (ret == -1)
                            {
                                strncpy(sendBuffer, "查询出错", sizeof(sendBuffer));
                                send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                                printf("%s\n", sendBuffer);
                                memset(sendBuffer, 0, sizeof(sendBuffer));
                            }
                            /*结果为空*/
                            else if (ret == 0)
                            {
                                strncpy(sendBuffer,"还没有群聊", sizeof(sendBuffer));
                                send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                                printf("%s\n", sendBuffer);
                                memset(sendBuffer, 0, sizeof(sendBuffer));
                            }
                            /*结果不为空*/
                            if (ret == 1)
                            {
                                memset(sendBuffer, 0, sizeof(sendBuffer));
                                strncpy(sendBuffer, str_groupNameTra, sizeof(sendBuffer));
                                send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                                printf("%s\n", sendBuffer);
                                memset(sendBuffer, 0, sizeof(sendBuffer));
                            }


                            memset(sendBuffer, 0, sizeof(sendBuffer)); 
                            strncpy(sendBuffer, "1、以上为群聊信息,请输入群聊名进行聊天2、建群3、返回上一级", sizeof(sendBuffer));     
                            send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);                
                            memset(sendBuffer, 0, sizeof(sendBuffer)); 

                            memset(recvBuffer, 0, sizeof(recvBuffer));
                            ret = recv(acceptfd, recvBuffer, sizeof(recvBuffer), 0);
                            if (ret == 0)
                            {
                                printf("客户端%d关闭\n", acceptfd);
                                /*调用退出登录代码 to do*/
                                close(acceptfd);
                                return NULL;
                            }

                            if (!strncmp(recvBuffer, "1", sizeof(recvBuffer)))//输入群聊名称进行聊天
                            {
                                
                                
                                memset(recvBuffer, 0, sizeof(recvBuffer));
                                continue;
                            }                    
                            else if (!strncmp(recvBuffer, "2", sizeof(recvBuffer)))//建群
                            {
                                memset(recvBuffer, 0, sizeof(recvBuffer));
                                char buffer[BUFFER_SIZE];
                                memset(buffer, 0, sizeof(buffer));
                                /* 检查该客户有没有好友*/
                                if (client == NULL)
                                {
                                    memset(sendBuffer, 0, sizeof(sendBuffer));
                                    strncpy(sendBuffer, "您暂时没有好友无法建群,返回上一级", sizeof(sendBuffer));
                                    send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                                    memset(sendBuffer, 0, sizeof(sendBuffer));
                                    printf("您暂时没有好友无法建群,返回上一级\n");
                                    sleep(3);
                                    break;
                                }
                                else
                                {
 
                                     /*有好友将好友的信息发给该客户*/
                                    balanceBinarySearchTreeInOrderTravel(client, buffer); //这个应该写在服务器上在服务其中查询好友的列表
                                    send(acceptfd, buffer, sizeof(buffer), 0);
                                   
                                }

                                while (1)
                                {
                                    

                                    memset(sendBuffer, 0, sizeof(sendBuffer));
                                    strncpy(sendBuffer, "请选择1、步骤一填写群名2、退出返回上一级", sizeof(sendBuffer));
                                    send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                                    memset(sendBuffer, 0, sizeof(sendBuffer));

                                    memset(recvBuffer, 0, sizeof(recvBuffer));
                                    ret = recv(acceptfd, recvBuffer, sizeof(recvBuffer), 0);
                                    if (ret == 0)
                                    {
                                        printf("客户端%d关闭\n", acceptfd);
                                        /*调用退出登录代码 to do*/
                                        close(acceptfd);
                                        return NULL;
                                    }
                                    /*填写群名*/
                                    if (!strncmp(recvBuffer, "1", sizeof(recvBuffer)))
                                    {
                                        memset(recvBuffer, 0, sizeof(recvBuffer));
                                        recv(acceptfd, recvBuffer, sizeof(recvBuffer), 0);
                                        /*创建群名 成功返回1 失败返回0*/
                                        ret = createGroupName(recvBuffer, conn, Message);
                                        
                                        /*创建群名成功*/
                                        if (ret == 1)
                                        {
                                            strncpy(groupChatInfo->groupChatName, recvBuffer, sizeof(groupChatInfo->groupChatName));
                                            memset(recvBuffer, 0, sizeof(recvBuffer));
                                            char group_buffer[BUFFER_SIZE];
                                            memset(group_buffer, 0, sizeof(group_buffer));

                                            memset(sendBuffer, 0, sizeof(sendBuffer));
                                            strncpy(sendBuffer, "创建群名成功", sizeof(sendBuffer));
                                            
                                            send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                                            printf("%s\n", sendBuffer);
                                            
                                            memset(sendBuffer, 0, sizeof(sendBuffer));

                                            /*输入好友账号拉取群聊*/
                                        while (1)
                                        {    
                                            
                                            memset(recvBuffer, 0, sizeof(recvBuffer));
                                            recv(acceptfd, recvBuffer, sizeof(recvBuffer), 0);
                                            //判断好友是否在树里
                                            
                                            chatRoomMessage * node = (chatRoomMessage *)malloc(sizeof(chatRoomMessage));
                                            node->name = (char *)malloc(NAMESIZE);
                                            strncpy(node->name, recvBuffer, NAMESIZE);
                                            printf("686---%s---name:%s--\n", recvBuffer,node->name);
                                            
                                            /*查找该人员昵称是否为你的好友*/
                                            if ((!chatRoomSelect(client, node)) && 
                                            pullGroupMembers(conn, node->name, Message, groupChatInfo))
                                            {
                                                groupChatInfo->membersName = node->name;
                                                printf("成员名%s\n", groupChatInfo->membersName);
                                                memset(sendBuffer, 0, sizeof(sendBuffer));
                                                strncpy(sendBuffer, "他是你的好友,入群成功", sizeof(recvBuffer));
                                                send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                                                memset(sendBuffer, 0, sizeof(sendBuffer));


                                            }
                                            else
                                            {
                                                memset(sendBuffer, 0, sizeof(sendBuffer));
                                                strncpy(sendBuffer, "出现错误，入群失败", sizeof(recvBuffer));
                                                send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                                                memset(sendBuffer, 0, sizeof(sendBuffer));
                                                
                                                
                                            }

                                            memset(recvBuffer, 0, sizeof(recvBuffer));
                                            recv(acceptfd, recvBuffer, sizeof(recvBuffer), 0);
                                            if (!strncmp(recvBuffer, "1", sizeof(recvBuffer)))
                                            {
                                                memset(recvBuffer, 0, sizeof(recvBuffer));
                                                break;
                                            }
                                            else if (!strncmp(recvBuffer, "2", sizeof(recvBuffer)))
                                            {
                                                memset(recvBuffer, 0, sizeof(recvBuffer));
                                                continue;
                                            }
                                                                //this
                                        }

                                            

                                        }
                                        /*失败*/
                                        else if (ret == 0)
                                        {
                                            printf("651----\n");
                                            memset(sendBuffer, 0, sizeof(sendBuffer));
                                            strncpy(sendBuffer, "重名/创建群名失败", sizeof(sendBuffer));
                                            send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                                            printf("%s", sendBuffer);
                                            memset(sendBuffer, 0, sizeof(sendBuffer));
                                            continue;
                                        }



                                    }

                                    /*退出返回上一级*/
                                    else if (!strncmp(recvBuffer, "2", sizeof(recvBuffer)))
                                    {
                                        memset(recvBuffer, 0, sizeof(recvBuffer));
                                        break;
                                    }
                                    else
                                    {
                                        memset(recvBuffer, 0, sizeof(recvBuffer));
                                        printf("输入有误，重新输入\n");
                                        continue;
                                    }
                                    

                                  
                                }
                   
                            }
                            else if (!strncmp(recvBuffer, "3", sizeof(recvBuffer)))//返回上一级(主界面)
                            {
                                memset(recvBuffer, 0, sizeof(recvBuffer));
                                break;
                            }
                            else 
                            {
                                memset(recvBuffer, 0, sizeof(recvBuffer));
                                printf("无效的输入请重新输入\n");
                                continue;
                            }



                    //开一个线程处理群聊
                    pthread_create(&tid_groupchat, NULL, handle_group_chat, NULL);
                    /*群聊 to do..*/
                    }
                    }
                 else if (!strncmp(recvBuffer, "2", sizeof(recvBuffer)))
                {
                    // memset(recvBuffer, 0, sizeof(recvBuffer));
                    // recv(acceptfd, recvBuffer, sizeof(recvBuffer), 0);
                    char buffer[BUFFER];
                    memset(buffer, 0, sizeof(buffer));
                    /* 检查该客户有没有好友*/
                    if (client == NULL)
                    {
                        memset(sendBuffer, 0, sizeof(sendBuffer));
                        strncpy(sendBuffer, "您暂时没有好友无法聊天,返回上一级", sizeof(sendBuffer));
                        send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                        memset(sendBuffer, 0, sizeof(sendBuffer));
                        printf("您暂时没有好友无法聊天,返回上一级\n");
                        sleep(3);
                        continue;
                    }
                    else
                    {
                        /*有好友将好友的信息发给该客户*/
                        balanceBinarySearchTreeInOrderTravel(client, buffer); //这个应该写在服务器上在服务其中查询好友的列表
                        printf("527--- %s\n", buffer);
                        send(acceptfd, buffer, sizeof(buffer), 0);
                    }

                    memset(buffer, 0, sizeof(buffer));
                    recv(acceptfd, recvBuffer, sizeof(recvBuffer), 0);
                    if (!strncmp(recvBuffer, "1", sizeof(recvBuffer)))
                    {
                        memset(recvBuffer, 0, sizeof(recvBuffer));
                        recv(acceptfd, recvBuffer, sizeof(recvBuffer), 0);
                        chatRoomMessage * node = (chatRoomMessage *)malloc(sizeof(chatRoomMessage));
                        node->name = (char *)malloc(NAMESIZE);
                        strncpy(node->name, recvBuffer, NAMESIZE);
                        printf("540---%s---name:%s--\n", recvBuffer,node->name);
                        
                        /*查找该人员昵称是否为你的好友*/
                        if (!chatRoomSelect(client, node))
                        {
                            strncpy(sendBuffer, "他是你的好友", sizeof(recvBuffer));
                            send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                          
                        }
                    }
                    
                    printf("587---recvBuffer:%s\n", recvBuffer);
                    char friendName[NAMESIZE];
                    memset(friendName, 0, NAMESIZE);
                    strncpy(friendName, recvBuffer, NAMESIZE);
                    memset(recvBuffer, 0, sizeof(recvBuffer));
                    /*判断好友是否在线 在线返回好友套接字fd */
                    int ret = searchFriendIfOnline(hashHandle->onlineTable, friendName);
                        printf("594--ret:%d\n", ret);
                        memset(recvBuffer, 0, sizeof(recvBuffer));
                        if (ret > 0)   /*此时好友在线*/
                        {
                            memset(sendBuffer, 0, sizeof(sendBuffer));  /*清空缓存区*/
                            strncpy(sendBuffer, "好友在线", sizeof(sendBuffer));
                            send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                            memset(sendBuffer, 0, sizeof(sendBuffer)); 
                           
                            memset(buffer, 0, sizeof(buffer));
                            recv(acceptfd, recvBuffer, sizeof(recvBuffer), 0);
                            printf("605----recvBuffer:%s\n", recvBuffer);
                            char myAccountNumber[NAMESIZE];
                            memset(myAccountNumber, 0, ACCOUNTNUMBER);
                            strncpy(myAccountNumber, recvBuffer, ACCOUNTNUMBER);
                            printf("609----myAccountNumber:%s\n", myAccountNumber);

                            snprintf(buffer, sizeof(buffer), "SELECT name from chatRoom WHERE accountNumber = '%s'", myAccountNumber);
                            if (mysql_query(conn, buffer))
                            {
                                printf("数据库错误\n");    
                                exit(-1);
                            }
                            else        /*需要加一个将查询出的结果放到数组中，再放入好友数据库中*/
                            {
                                MYSQL_RES *res = mysql_use_result(conn);
                                memset(buffer, 0, sizeof(buffer));
                                if (res != NULL) 
                                {
                                    MYSQL_ROW row;
                                    if ((row = mysql_fetch_row(res)) != NULL) 
                                    {
                                        
                                        snprintf(buffer, sizeof(buffer), "%s", row[0]);
                                        printf("628----buffer:%s\n", buffer);


                                    }
                                    mysql_free_result(res);  // 释放查询结果集
                                }
                            }
                            printf("635----buffer:%s\n", buffer);

                            /*自己的名字*/
                            send(acceptfd, buffer, sizeof(buffer), 0);
                            printf("buffer:%s\n",buffer);
                            /*设置sock为非阻塞状态使读写非阻塞*/   
                        //    fcntl(acceptfd, F_SETFL, O_NONBLOCK);
                            
                            // fcntl(ret, F_SETFL, O_NONBLOCK);
                            memset(buffer, 0, sizeof(buffer));
                            /*发送消息给好友*/
                            while (1)
                            {   
                                memset(buffer, 0, sizeof(buffer));
                                memset(recvBuffer, 0, sizeof(recvBuffer));
                                printf("recvbuffer:%s",recvBuffer);
                                if (recv(acceptfd, recvBuffer, sizeof(recvBuffer), 0) > 0)
                                {
                                    printf("652--读:%s",recvBuffer);
                                    if (send(ret, recvBuffer, sizeof(recvBuffer), 0) == -1)
                                    {
                                        break;
                                    }
                                    
                                }
                                
                                printf("660 recvbuffer:%s",recvBuffer);
                                
                                if (recv(ret, buffer, sizeof(buffer), 0))
                                {
                                    printf("663--读:%s",buffer);

                                    if (send(acceptfd, buffer, sizeof(buffer), 0) == -1)
                                    {
                                        break;
                                    }
                                }
                                
                                
                                
                                 
                            }
                            
                            
                            
                        }
                        if (ret == -1)
                        {
                            memset(sendBuffer, 0, sizeof(sendBuffer));  /*清空缓存区*/
                            strncpy(sendBuffer, "此时好友不在线", sizeof(sendBuffer));
                            send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                            memset(sendBuffer, 0, sizeof(sendBuffer)); 
                            continue;
                        }
                    }
                }   
            }
            else if (!strncmp(recvBuffer, "3", sizeof(recvBuffer)))
            {
                //删除好友
            }
            else if (!strncmp(recvBuffer, "6", sizeof(recvBuffer)))
            {
                  
                    memset(sendBuffer, 0, sizeof(sendBuffer));
                    strncpy(sendBuffer, "用户退出登录", sizeof(sendBuffer));
                    printf("%s\n", sendBuffer);

                    send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                
                    memset(sendBuffer, 0, sizeof(sendBuffer));
#if 1
                int delete_name = getAsciiSum(Message->name);
                hashTableDelAppointKey(hashHandle->onlineTable, delete_name);/*删除在线列表中该用户的信息*/
                printf("客户端退出\n");
#endif
                    break;
            }
            else if (!strncmp(recvBuffer, "X", sizeof(recvBuffer)))
            {
                    //注销登录
            }
            else
            {
                memset(recvBuffer, 0, sizeof(recvBuffer));

                printf("输入有误，请重新选择\n");
#if 0
                    memset(sendBuffer, 0, sizeof(sendBuffer));
                    strncpy(sendBuffer, "输入有误，请重新选择", sizeof(sendBuffer));
                    send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
                    printf("%s\n", sendBuffer);
                    memset(sendBuffer, 0, sizeof(sendBuffer));
#endif
                continue;

            

            }
        
        }   

            
        
        
        // ... 原先的代码 ...
    
    

    // 原先的代码结束

    close(acceptfd);  // 处理结束后关闭acceptfd
    // pthread_exit(NULL);
    return NULL;
}

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
    groupChat * groupChatInfo = NULL;

    chatRoomInit(&Message, &groupChatInfo, &friendMessage, &obj, &Info, &client, &online, &conn, compareFunc, compareFunc1, printStruct, node, &onlineTable);

    threadpool_t pool;
    int minThreads;
    int maxThreads;
    int queueCapacity;
    pthread_t tid;
    
    threadPoolInit(&pool, minThreads, maxThreads, queueCapacity);

    fdHash * hashHandle = (fdHash *)malloc(sizeof(fdHash));
    memset(hashHandle, 0, sizeof(fdHash));
    hashHandle->onlineTable = (HashTable *)malloc(sizeof(HashTable));
    memset(hashHandle->onlineTable, 0, sizeof(HashTable));

    
    // pthread_mutex_t mutex_server;
    // pthread_mutex_init(&mutex_server, NULL);
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

            int acceptfd = accept(sockfd, NULL, NULL);
            if (acceptfd == -1)
            {
                perror("accpet error");
                break;
            }
            hashHandle->onlineTable = onlineTable;
            hashHandle->sockfd =acceptfd;
            // pthread_mutex_lock(&mutex_server);
            // ret = pthread_create(&tid, NULL, handleClient, (void *)&acceptfd);
            threadPoolAddTask(&pool, (void *)handleClient, (void *)hashHandle);
            // pthread_mutex_unlock(&mutex_server); 
    }


    /* 关闭文件描述符 */
    close(sockfd);






    return 0;
}