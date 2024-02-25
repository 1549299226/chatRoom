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
#include <fcntl.h>

#define BUFFER_SIZE 1024
#define SERVER_PORT 9999
#define SERVER_IP "127.0.0.1"

#define ACCOUNTNUMBER 6
#define NAMESIZE 12

#define TIME_SIZE 100

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

void exitChat(int * arg)
{
    *arg = 1;
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

void * private_chat(void * arg)
{
    pthread_detach(pthread_self());
    int sockfd = ((chatOTO *)arg)->sockfd;
    char recvBuffer[BUFFER_SIZE];
    memset(recvBuffer, 0, sizeof(recvBuffer));
    chatContent * closedChat = (chatContent *)malloc(sizeof(chatContent));
    memset(closedChat, 0, sizeof(chatContent));
    closedChat->content = (char *)malloc(BUFFER_SIZE);
    memset(closedChat->content, 0, BUFFER_SIZE);
    closedChat->friendName = (char *)malloc(NAMESIZE);
    memset(closedChat->friendName, 0, NAMESIZE);
    closedChat->chatTime = 0;

    closedChat->myName = (char *)malloc(NAMESIZE);
    memset(closedChat->myName, 0, NAMESIZE);
    
    json_object * obj = NULL;
    char *dateStr = (char *)malloc(TIME_SIZE);
    while (((chatOTO *)arg)->otoBuffer == 1)
    {
        
        memset(closedChat->friendName, 0, NAMESIZE);
        closedChat->chatTime = 0;
        memset(closedChat->content, 0, BUFFER_SIZE);
        memset(closedChat->myName, 0, NAMESIZE);
        memset(dateStr, 0, sizeof(dateStr));
        // printf("100 recvBuffer:%s\n", recvBuffer);

        if (recv(sockfd, recvBuffer, sizeof(recvBuffer), 0) == -1)
        {
            perror("recv");
            break;
        }
        if (strlen(recvBuffer) == 1 && recvBuffer[0] == 27)
        {
            memset(recvBuffer, 0, sizeof(recvBuffer));
            printf("关闭客户端的读\n");
            break;
        }
        
        //printf("105 recvBuffer:%s\n", recvBuffer);
        if (chatRoomObjAnalyzeContent(recvBuffer, closedChat, obj))
        {
            printf("json转聊天结构体失败\n");
            break;
        }
        memset(dateStr, 0, sizeof(dateStr));

        /*将时间戳转换成表示日期的字符串*/
        dateStr = ctime(&closedChat->chatTime);
        struct tm *localTime = localtime(&closedChat->chatTime);
        char formattedTime[TIME_SIZE];
        memset(formattedTime, 0, TIME_SIZE);

        /*自定义时间的表示*/
        strftime(formattedTime, sizeof(formattedTime), "%Y-%m-%d %H:%M:%S", localTime);
        printf("friendname:%s, time:%s \n %s\n", closedChat->myName, formattedTime, closedChat->content);
        
         
    }
    pthread_exit(NULL);
    
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
    
    pthread_t tid_oto;
    chatRoomMessage *Message = NULL;
    json_object *obj = NULL;
    Friend *Info = NULL;
    MYSQL *conn = NULL;
    friendNode *node = NULL;
    Friend *client = NULL;
    Friend * online = NULL;
    chatContent * friendMessage = NULL;
    groupChat * groupChatInfo = NULL;

    chatHash * onlineHash = (chatHash *)malloc(sizeof(chatHash));
    onlineHash->hashName = (char *)malloc(NAMESIZE); 
    onlineHash->sockfd = 0;
    chatRoomInit(&Message, &groupChatInfo, &friendMessage, &obj, Info, client, online, &conn, existenceOrNot, printStruct, node); /*初始化*/

     
    
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
        //登陆注册
        while (1)
        {       
            fristInterface();
            scanf("%s", flag);
            send(sockfd, flag, sizeof(flag), 0); 
            if (!strncmp(flag, "1", sizeof(flag)))
            {
                
                   //写入选项

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

                    memset(flag, 0, sizeof(flag));

                    continue;
                }
            }
            else if(!strncmp(flag, "2", sizeof(flag)))
            {   
                system("clear");
                //send(sockfd, flag, sizeof(flag), 0);   //写入选项      
                recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);  //读取返回的
                printf("%s\n", recvBuffer);
                usleep(500);
                memset(recvBuffer, 0, sizeof(recvBuffer));
                memset(userBuf, 0, sizeof(userBuf));

                chatRoomClientLogIn(userBuf,Message, obj);
                /*写入账号密码*/
                send(sockfd, userBuf, sizeof(userBuf), 0);

                // recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
                // printf("%s\n", recvBuffer);
                memset(recvBuffer, 0, sizeof(recvBuffer));
             
                recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
                if (!strncmp(recvBuffer, "登录成功", sizeof(recvBuffer) -1))
                {
                    printf("登录成功\n");
                    sleep(2);
                    enterInterface();
                    memset(flag, 0, sizeof(flag));

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
                /*死循环*/
                continue;
           
            }
        }
    
        //主界面
        while (1)
        {
            mainInterface();
            printf("请选择\n");
            scanf("%s", flag);
            send(sockfd, flag, sizeof(flag), 0);//发送选择给服务端
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
                        memset(flag, 0, sizeof(flag));
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
                        memset(flag, 0, sizeof(flag));
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
                                    sleep(3);
                                    memset(flag, 0, sizeof(flag));
                                    continue;
                                }
                                else if (!strncmp(recvBuffer, "添加好友成功", sizeof(recvBuffer)))
                                {
                                    printf("%s\n", recvBuffer);
                                    sleep(3);
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
                    else if (!strncmp(flag, "3", sizeof(flag)))
                    {
                        memset(flag, 0, sizeof(flag));
                        break;
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

                /*写入选项*/

                while (1)
                {
                    system("clear");

                    /*读取选项*/
                    /*请选择1、群聊 2、私聊 3、返回上一级*/

                    memset(recvBuffer, 0, sizeof(recvBuffer));
                    recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
                    printf("%s\n", recvBuffer); 

                    memset(recvBuffer, 0, sizeof(recvBuffer));
                    printf("请输入选项\n");
                    scanf("%s", flag);
                    send(sockfd, flag, sizeof(flag), 0);

                    if (!strncmp(flag, "1", sizeof(flag)))/*群聊功能*/
                    {
                        memset(flag, 0, sizeof(flag));
                        /*群聊 to do..*/
                        while (1)
                        {
                            system("clear");

                            memset(recvBuffer, 0, sizeof(recvBuffer));
                            recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
                            if (!strncmp(recvBuffer, "查询出错", sizeof(recvBuffer)))
                            {
                                printf("%s\n", recvBuffer);
                                memset(recvBuffer, 0, sizeof(recvBuffer));
                            }
                            else if (!strncmp(recvBuffer, "还没有群聊", sizeof(recvBuffer)))
                            {
                                printf("%s\n", recvBuffer);
                                memset(recvBuffer, 0, sizeof(recvBuffer));
                            }
                            else 
                            {
                                printf("%s\n", recvBuffer);
                                memset(recvBuffer, 0, sizeof(recvBuffer));
                            }

                            memset(recvBuffer, 0, sizeof(recvBuffer));
                            recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
                            printf("%s\n", recvBuffer);
                            memset(recvBuffer, 0, sizeof(recvBuffer));
                            /*1、请输入群聊名进行聊天2、建群3、返回上一级*/
                            scanf("%s", flag);
                            send(sockfd, flag, sizeof(flag), 0);
                            /*输入群聊名称进行聊天*/
                            if (!strncmp(flag, "1", sizeof(flag)))
                            {
                                printf("请输入群名\n");
                                memset(sendBuffer, 0, sizeof(sendBuffer));
                                scanf("%s", sendBuffer);
                                send(sockfd, sendBuffer, sizeof(sendBuffer), 0);

                                memset(recvBuffer, 0, sizeof(recvBuffer));
                                recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
                                printf("群成员：%s\n", recvBuffer);
                                
                                memset(recvBuffer, 0, sizeof(recvBuffer));
                                recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
                                if (!strncmp(recvBuffer, "请输入聊天内容", sizeof(recvBuffer)))
                                {
                                    printf("%s", recvBuffer);
                                    scanf("%s", groupChatInfo->groupChatContent);
                                    memset(sendBuffer, 0, sizeof(sendBuffer));
                                    send(sockfd, groupChatInfo->groupChatContent, sizeof(groupChatInfo->groupChatContent), 0);

                                    memset(recvBuffer, 0, sizeof(recvBuffer));
                                    recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
                                     
                                    printf("你收到一条群聊消息:%s\n", recvBuffer);
                                    memset(recvBuffer, 0, sizeof(recvBuffer));

                                }

                                memset(flag, 0, sizeof(flag));
                                continue;

                            }
                            /*建群*/
                            else if (!strncmp(flag, "2", sizeof(flag)))
                            {

                                char tmp_friendinfo[BUFFER_SIZE];
                                memset(tmp_friendinfo, 0, sizeof(tmp_friendinfo));

                                memset(flag, 0, sizeof(flag));

                                memset(recvBuffer, 0, sizeof(recvBuffer));
                                recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
                                if (!strncmp(recvBuffer, "您暂时没有好友无法建群,返回上一级", sizeof(recvBuffer)))
                                {
                                    printf("%s\n", recvBuffer);
                                    memset(recvBuffer, 0, sizeof(recvBuffer));
                                    break;
                                }
                                else
                                {
                                    printf("以下是所有好友的信息:\n");
                                    /*拷贝好友信息备用*/
                                    memcpy(tmp_friendinfo, recvBuffer, sizeof(tmp_friendinfo));
                                    printf("%s\n", recvBuffer);
                                }
                                while (1)
                                {
                                    system("clear");
                                    printf("以下是所有好友的信息:\n");
                                    printf("%s\n", tmp_friendinfo);
                                    memset(recvBuffer, 0, sizeof(recvBuffer));

                                    recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
                                    /*请选择1、填写群名2、退出返回上一级*/
                                    printf("%s\n", recvBuffer);
                                    memset(recvBuffer, 0, sizeof(recvBuffer));
                                    /*输入选项*/
                                    scanf("%s", flag);

                                    memset(sendBuffer, 0, sizeof(sendBuffer));
                                    send(sockfd, flag, sizeof(sendBuffer), 0);
                                    /*填写群名*/
                                    if (!strncmp(flag, "1", sizeof(flag)))
                                    {
                                        memset(flag, 0, sizeof(flag));
                                        printf("请输入群名\n");
                                        scanf("%s", groupChatInfo->groupChatName);

                                        memset(sendBuffer, 0, sizeof(sendBuffer));
                                        strncpy(sendBuffer, groupChatInfo->groupChatName, sizeof(sendBuffer));
                                        send(sockfd, sendBuffer, sizeof(sendBuffer), 0);
                                        memset(sendBuffer, 0, sizeof(sendBuffer));

                                        memset(recvBuffer, 0, sizeof(recvBuffer));
                                        recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
                                        if (!strncmp(recvBuffer, "创建群名成功", sizeof(recvBuffer)))
                                        {
                                            printf("%s\n", recvBuffer);
                                            sleep(2);
                                            memset(recvBuffer, 0, sizeof(recvBuffer));
                                            while (1)
                                            {
                                                printf("请输入你要邀请的好友名称\n");
                                                scanf("%s", groupChatInfo->membersName);
                                                memset(sendBuffer, 0, sizeof(sendBuffer));
                                                send(sockfd, groupChatInfo->membersName, sizeof(sendBuffer), 0);
                                                memset(sendBuffer, 0, sizeof(sendBuffer));

                                                memset(recvBuffer, 0, sizeof(recvBuffer));
                                                recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
                                                if (!strncmp(recvBuffer, "他是你的好友,入群成功", sizeof(recvBuffer)))
                                                {
                                                    printf("%s\n", recvBuffer);
                                                    memset(recvBuffer, 0, sizeof(recvBuffer));

                                                }
                                                else if (!strncmp(recvBuffer, "出现错误，入群失败", sizeof(recvBuffer)))
                                                {
                                                    printf("%s\n", recvBuffer);
                                                    memset(recvBuffer, 0, sizeof(recvBuffer));
                                                    
                                                }

                                            printf("选择接下来的操作:1、退出2、继续添加好友\n");
                                            memset(flag, 0, sizeof(flag));
                                            scanf("%s", flag);

                                            send(sockfd, flag, sizeof(sendBuffer), 0);
                                            if (!strncmp(flag, "1", sizeof(flag)))
                                            {
                                                memset(flag, 0, sizeof(flag));
                                                break;
                                            }
                                            else if (!strncmp(flag, "2", sizeof(flag)))
                                            {
                                                memset(flag, 0, sizeof(flag));
                                                continue;   
                                            }
                                            }
                                            
                                        }
                                        else if (!strncmp(recvBuffer, "重名/创建群名失败", sizeof(recvBuffer)))
                                        {
                                            printf("%s\n", recvBuffer);
                                            sleep(1);
                                            memset(recvBuffer, 0, sizeof(recvBuffer));
                                            continue;
                                        }

                                        
                                        


                                    }
                                    
                                    /*退出返回上一级*/
                                    else if (!strncmp(flag, "2", sizeof(flag)))
                                    {
                                        memset(flag, 0, sizeof(flag));
                                        break;
                                    }
                                    else
                                    {
                                        memset(flag, 0, sizeof(flag));
                                        printf("输入有误，重新输入\n");
                                        continue;
                                    }

                                    
                                }
                                

                                
                            }
                            /*返回上一级*/
                            else if (!strncmp(flag, "3", sizeof(flag)))
                            {
                                memset(flag, 0, sizeof(flag));
                                break;

                            }
                            else
                            {
                                memset(flag, 0, sizeof(flag));
                                printf("无效的输入请重新输入\n");
                                sleep(2);
                                continue;
                            }

                        }


 
                }
                /*私聊*/
                   /*私聊*/
                else if (!strncmp(flag, "2", sizeof(flag)))
                {
                    memset(flag, 0, sizeof(flag));
                    memset(recvBuffer, 0, sizeof(recvBuffer));
                    recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
                    if (!strncmp(recvBuffer, "您暂时没有好友无法聊天,返回上一级", sizeof(recvBuffer)))
                    {
                        printf("%s\n", recvBuffer);
                        memset(recvBuffer, 0, sizeof(recvBuffer));
                        continue;
                    }
                    else
                    {
                        printf("以下是所有好友的信息:\n");
                        printf("%s\n", recvBuffer);
                    }
                    

                    
                    while (1)
                    {
                        printf("1、输入私聊对象的名字进行聊天\n");
                        printf("2、退出返回上一界面\n");
                        scanf("%s", flag);
                        send(sockfd, flag, sizeof(flag), 0);

                        if (!strncmp(flag, "1", sizeof(flag)))
                        {
                            memset(flag, 0, sizeof(flag));
                            
                            printf("请输入要聊天的好友姓名\n");
                            scanf("%s", friendMessage->friendName);
                            /*先清零缓冲区*/
                            send(sockfd, friendMessage->friendName, NAMESIZE, 0);
                            memset(sendBuffer, 0, sizeof(sendBuffer));

                            recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
                            printf("%s\n", recvBuffer);
                            if (!strncmp(recvBuffer, "他是你的好友", sizeof(recvBuffer))) /*好友存在时*/
                            {
#if 1 
                                /*清空缓冲区*/
                                memset(recvBuffer, 0, sizeof(recvBuffer));
                                    /*接收好友是否在线的信息*/
                                recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
                                
                                chatOTO oto;
                                oto.sockfd = sockfd;
                                oto.otoBuffer = 1;
                                    // 接收到数据成功
                                //recvBuffer[ret] = '\0';  // 在接收到的数据末尾添加字符串结束符
                                if (!strncmp(recvBuffer, "好友在线", sizeof(recvBuffer)))
                                {
                                    printf("%s\n",recvBuffer);

                                    send(sockfd, Message->accountNumber , ACCOUNTNUMBER, 0);
                                    memset(recvBuffer, 0, sizeof(recvBuffer));
                                    recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
                                    strncpy(friendMessage->myName, recvBuffer, NAMESIZE); 
                                    
                                    char buffer[BUFFER_SIZE];
                                    memset(buffer, 0, sizeof(buffer));
                                    pthread_create(&tid_oto, 0, private_chat, (void *)&oto);
                                    while (1)
                                    {
                                        memset(sendBuffer, 0, sizeof(sendBuffer));
                                        memset(buffer, 0, sizeof(buffer));
                                        friendMessage->chatTime = time(NULL);
                                        while (getchar() != '\n');
                                        scanf("%s", sendBuffer);
                                        
                                        strncpy(friendMessage->content, sendBuffer, sizeof(sendBuffer)); 
                                        if (chatRoomObjConvertContent(buffer, friendMessage, obj))
                                        {
                                            printf("聊天结构体转json失败\n");
                                            break;
                                        }
                            
                                        
                                        if (strlen(sendBuffer) == 1 && sendBuffer[0] == 27)
                                        {
                                            printf("客户端的写已关闭\n");
                                            oto.otoBuffer = 0;
                                            send(sockfd, sendBuffer, sizeof(sendBuffer), 0);
                                            memset(sendBuffer, 0, sizeof(sendBuffer));
                                            break;
                                        }
                                        if (send(sockfd, buffer, sizeof(buffer), 0) == -1)
                                        {
                                            perror("send");
                                            break;
                                    
                                        }
                                        printf("585--长度:%ld\n", strlen(sendBuffer));
                                    }
                                    
                                    break;                                       
                                }

                                if (!strncmp(recvBuffer, "此时好友不在线", sizeof(recvBuffer)))
                                {
                                    memset(recvBuffer, 0, sizeof(recvBuffer));
                                    
                                    printf("此时好友不在线, 设计为不通信，返回上一级\n");
                                    break;
                                }
                                
                                
#endif
                            }
                            else
                            {
                                printf("他不是你的好友， 返回上一级\n");
                                break;

                            }
                            

                        }
                        else if (!strncmp(flag, "2", sizeof(flag)))
                        {
                            memset(flag, 0, sizeof(flag));
                            break;
                        }
                        else 
                        {
                            memset(flag, 0, sizeof(flag));
                            printf("无效的输入，请重新输入\n");
                            mainInterface();
                            continue;
                        }

                    }

                }

                }
            }

            else if (!strncmp(flag, "3", sizeof(flag)))//删除好友
            {
                //删除好友
                recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
                printf("%s\n", recvBuffer)
                printf("输入你想要删除的好友\n");
                scanf("%s", sendBuffer);
                send(sockfd, sendBuffer, sizeof(sendBuffer), 0);
                memset(sendBuffer, 0, sizeof(sendBuffer));
                recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
                if (!strncmp(recvBuffer, "他不是你的好友", sizeof(recvBuffer)))
                {
                    printf("他不是你的好友");
                    continue;
                }
                else if (!strncmp(recvBuffer, "他是你的好友", sizeof(recvBuffer)))
                {
                    memset(recvBuffer, 0, sizeof(recvBuffer));
                    
                    recv(sockfd,recvBuffer, sizeof(recvBuffer), 0);
                    printf("%s", recvBuffer);
                    continue;

                }
                
                
                
            }
            else if (!strncmp(flag, "6", sizeof(flag)))//退出登录
            {

                //退出登录
                memset(flag, 0, sizeof(flag));
 
                
                recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
                printf("%s\n", recvBuffer);
                sleep(3);
                memset(recvBuffer, 0, sizeof(recvBuffer));
                
                logoutCleanup(Message, friendMessage, obj, conn, node);
                free(flag);
                quitChatInterface();//进入退出界面

                break;


            }
            else if (!strncmp(flag, "X", sizeof(flag)))//注销登录
            {
                //注销登录
            }
            else
            {
                memset(flag, 0, sizeof(flag));
                printf("输入有误，请重新选择\n");
                continue;

                

            }
        }
        
    
    
    }
    close(sockfd);
    return 0;
}
