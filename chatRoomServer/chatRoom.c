#include <stdio.h>
#include "chatRoom.h"
#include <stdlib.h>
#include <error.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <mysql/mysql.h>
#include <sys/stat.h>
#include "hashtable.h"
#include <json-c/json_object.h>

#define PASSWORD_MAX 8  
#define PASSWORD_MIN 6
#define MAILSIZE 20
#define NAMESIZE 12
#define ACCOUNTNUMBER 6
#define FILE_PATH 50

/*数据库的宏*/
#define DBHOST "127.0.0.1"
#define DBUSER "root"
#define DBPASS "1"
#define DBNAME "chatRoom"


#define BUFFER_SIZE 100
#define MAX_ONLINE 50

enum FILE_STATUS
{
    PATH_ERR = -1,
    FILE_EXIT = 1,
};

enum CHOIVE
{
    ONE = 1,
    Two
};

static int fileEixt(const char * filePath);



/*输入地址的静态*/
static int inputPath (char * path);



static int accountRegistration(char * accountNumber , MYSQL * conn);    //判断账号是否合法

static int registrationPassword(char * password);       //判断密码是否合法

static int nameLegitimacy(char * name, MYSQL * conn);  //判断昵称的合法性

static int determineIfItExists(chatRoomMessage *Message, MYSQL * conn); //判断账号密码是否正确

int hashTableCompare(void *arg1, void *arg2)
{
    chatRoomMessage *idx1 = (chatRoomMessage *) arg1;
    chatRoomMessage *idx2 = (chatRoomMessage *) arg2;
    // char * idx1 = (char *)arg1;
    // char * idx2 = (char *)arg2;
    int result = 0;
    result = strcmp(idx1->name, idx2->name);

    return result;
}

/*初始化聊天室*/
int chatRoomInit(chatRoomMessage **Message, json_object **obj, Friend *Info, Friend *client, Friend * online, MYSQL **conn, int (*compareFunc)(ELEMENTTYPE val1, ELEMENTTYPE val2), int (*printFunc)(ELEMENTTYPE val), friendNode *node, HashTable ** onlineTable) /*先这些后面再加*/
{
    int ret = 0;

    *Message = (chatRoomMessage *)malloc(sizeof(chatRoomMessage));
    memset(*Message, 0, sizeof(Message));
    /*初始化姓名*/
    (*Message)->name = (char *)malloc(sizeof(char) * NAMESIZE);
    if ((*Message)->name == NULL)
    {
        return MALLOC_ERROR;
    }
    /*清楚脏数据*/
    bzero((*Message)->name, sizeof(char) * NAMESIZE);

    /*账号初始化*/
    (*Message)->accountNumber = (char *)malloc(sizeof(char) * ACCOUNTNUMBER);
    if ((*Message)->accountNumber == NULL)
    {
        return MALLOC_ERROR;
    }
    bzero((*Message)->accountNumber, sizeof(char) * ACCOUNTNUMBER);

    /*邮箱初始化*/
    (*Message)->mail = (char *)malloc(sizeof(char) * MAILSIZE);
    if ((*Message)->mail == NULL)
    {
        return MALLOC_ERROR;
    }
    bzero((*Message)->mail, sizeof(char) * MAILSIZE);

    /*密码初始化*/
    (*Message)->password = (char *)malloc(sizeof(char) * PASSWORD_MAX);
    if ((*Message)->password == NULL)
    {
        return MALLOC_ERROR;
    }
    bzero((*Message)->password, PASSWORD_MAX);

    // 创建一个json对象
    // 创建一个json对象
    *obj = (json_object*)malloc(sizeof(json_object*)); 
    *obj = json_object_new_object();
    if (obj == NULL) 
    {
        fprintf(stderr, "Failed to create JSON object\n");
        return -1;
    }
    // 将用户列表初始化
    balanceBinarySearchTreeInit(&Info, compareFunc, printFunc);

    balanceBinarySearchTreeInit(&client, compareFunc, printFunc);

    balanceBinarySearchTreeInit(&online, compareFunc, printFunc);
    // 初始化一个好友结点
    node = (friendNode *)malloc(sizeof(friendNode));
    if (node == NULL)
    {
        return MALLOC_ERROR;
    }
    bzero(node, sizeof(friendNode));

    /*结点内容初始化*/
    node->data = (chatRoomMessage *)malloc(sizeof(chatRoomMessage));
    if (node->data == NULL)
    {
        return MALLOC_ERROR;
    }
    bzero(node->data, sizeof(chatRoomMessage));
    node->height = 0;
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;

    /*初始化一个数据库*/
    *conn = mysql_init(NULL);        
    if (*conn == NULL)           /*判断是否正确*/
    {
        fprintf(stderr, "mysql_init failed\n");
        return MALLOC_ERROR;
    }

    /*连接数据库*/
    if (mysql_real_connect(*conn, DBHOST, DBUSER, DBPASS, NULL, 0, NULL, 0) == NULL) 
    {
        fprintf(stderr, "mysql_real_connect failed: %s\n", mysql_error(*conn));
        mysql_close(*conn);
        return MALLOC_ERROR;
    }
    
    /*创建数据库*/
    if (mysql_query(*conn, "CREATE DATABASE IF NOT EXISTS chatRoom"))
    {
        fprintf(stderr, "Error %u: %s\n", mysql_errno(*conn), mysql_error(*conn));
    } 
    
    /*打开数据库*/
    if (mysql_select_db(*conn, DBNAME)) 
    {
        fprintf(stderr, "Error %u: %s\n", mysql_errno(*conn), mysql_error(*conn));
        mysql_close(*conn);
        exit(1);
    } 

    char buffer[BUFFER_SIZE << 2];
    memset(buffer, 0, sizeof(buffer));
    //创建表
    snprintf(buffer, sizeof(buffer), "CREATE TABLE IF NOT EXISTS chatRoom "
                                 "(accountNumber VARCHAR(100) PRIMARY KEY, "
                                 "password VARCHAR(100) NOT NULL, "
                                 "name VARCHAR(100) NOT NULL, "
                                 "mail VARCHAR(100) NOT NULL)");
        
    if (mysql_query(*conn, buffer))
    {
        exit(-1);
    }

    
    int onlineFriNum = MAX_ONLINE;
    hashTableInit(onlineTable, onlineFriNum, hashTableCompare);
    
    printf("你好\n");



    return ret;
}



//判断账号是否正确,正确返回0，错误返回-1
static int accountRegistration(char * accountNumber , MYSQL * conn)        
{
    int len = 0;               //长度
    int flag = 0;            //标记
    int count = 0;           //计数器

    len = strlen(accountNumber);
    printf("accountNumber:%s\n", accountNumber);
    printf("---%d---\n", len);
    while (count < len)                          
    {
              /*保存账号长度*/

        if (accountNumber[count] > '9' || accountNumber[count] < '0')     /*判断是否满足账号要求*/
        {                                      
            memset(accountNumber, 0, sizeof(accountNumber));   /*不满足条件将内容归零，重新输入*/
            flag = 1;
        }
        count++;
    }  
    printf("----count:%d----\n", count); 
    if (count != ACCOUNTNUMBER || flag != 0)   /*判断账号长度是满足条件*/
    {  
        if (count != ACCOUNTNUMBER)
        {
            flag += 2;
        }
        if (flag == 2)
        {
            printf("账号长度不符合请重新输入\n");
        }
        else if (flag == 1)
        {
            printf("账号格式不符合请重新输入\n");
        }
        else if (flag == 3)
        {
            printf("账号长度与格式都不符合条件\n");
        }

        sleep(2);
        return -1;
    }

    
    /*创建一个缓冲区*/
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    /*将账号信息放进占位符中，放到缓存器buffer*/
    snprintf(buffer, sizeof(buffer), "SELECT accountNumber FROM chatRoom WHERE accountNumber = '%s'", accountNumber);
    
    printf("---%s\n", buffer);
    int ret = mysql_query(conn, buffer);
    
    /*判断该账号是否在数据库中*/
    if (ret == 0)       /*是否有该账号*/
    {
        MYSQL_RES* result = mysql_store_result(conn);
        int num_rows = mysql_num_rows(result);
        if (num_rows == 0) 
        {
        return 0;
        } 
        else 
        {
            printf("已有该账号\n");
            return -1;
        } 
    }
    else 
    {
        printf("查询失败: %d - %s\n", mysql_errno(conn), mysql_error(conn));
        return -1;
    }
    
    
}

//判断密码是否合法
static int registrationPassword(char * password)     //判断密码是否正确,正确返回0，错误返回-1
{
    int letter = 0;           // 记录是否有字母
    int figure = 0;           // 记录是否有数字
    int specialCharacter = 0; // 记录是否有特殊字符
    int len = strlen(password);
    int count = 0;
    int flag = 0;
    while (count < len)             //判断密码合法否
    {
        if (password[count] <= '9' && password[count] >= '0')       //判断是否有数字
        {
            figure++;
        }
        else if ((password[count] >= 'a' && password[count] <= 'z') || (password[count] >= 'A' && password[count] <= 'Z')) /*判断是否有字母*/
        {   
            letter++;
        }
        else if (password[count] != ' ' && password[count] != '\0')         /*判断是否有特殊字符*/
        {
            specialCharacter++;
        }
        count++;
    }
    if (count <= 6 || count >= 8 )
    {
        printf("密码长度不符\n");
        flag++;
    }
    if (figure == 0)
    {
        printf("密码格式不符，没有数字\n");
        flag++;
    }
    if (letter == 0)
    {
        printf("密码格式不符，没有字母\n");
        flag++;
    }
    if (specialCharacter == 0)
    {
        printf("密码格式不符，没有特殊字符\n");
        flag++;
    }

    if (flag != 0)
    {
        memset(password, 0, sizeof(password));
        return -1;
    }
    

    return 0;
}

//判断昵称的合法性, 正确返回0，错误返回-1
static int nameLegitimacy(char * name, MYSQL * conn)  
{
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    snprintf(buffer, sizeof(buffer), "SELECT name FROM chatRoom WHERE name = '%s'", name);

    /*返回值为零时查询成功*/
    int ret = mysql_query(conn, buffer);
    if (ret == 0)           /*判断其是否有该昵称*/
    {
        MYSQL_RES* result = mysql_store_result(conn);
        int num_rows = mysql_num_rows(result);
        if (num_rows == 0) 
        {
        return 0;
        } 
        else 
        {
            printf("已有该昵称\n");
            return -1;
        } 
    }
    else 
    {
        printf("查询失败: %d - %s\n", mysql_errno(conn), mysql_error(conn));
        return -1;
    }
    
    return 0;
}

/*注册*/    //注册成功返回0， 失败返回-1
int chatRoomInsert(chatRoomMessage *Message, MYSQL * conn) /*账号不能跟数据库中的有重复，昵称也是不可重复，通过账号算出一个key（用一个静态函数来计算），这个key便是ID是唯一的，密码要包含大写及特殊字符，最少八位，不然密码不符合条件，将注册好的信息放到数据库中*/
{
    
    int ret = 0;
    // chatRoomObjAnalyze(buffer, Message);
    printf("---conn:%p\n", conn);
    ret = accountRegistration(Message->accountNumber, conn);  /*判断账号是否合法*/
    if (ret == -1)      
    {
        return -1;
    }



    printf("请输入密码：(六到八位，包括大小写，特殊字符，及数字)\n");
    ret = registrationPassword(Message->password);
    if (ret == -1)
    {
        return -1; 
    }

    printf("请输入你的邮箱\n");
    printf("请输入昵称\n");
    ret = nameLegitimacy(Message->name, conn);
    if (ret == -1)
    {
        return -1;
    }
    
    char buf[BUFFER_SIZE];
    memset(buf, 0, sizeof(buf));

    snprintf(buf, sizeof(buf), "INSERT INTO chatRoom VALUES ('%s', '%s', '%s', '%s')", 
                Message->accountNumber, Message->password, Message->name, Message->mail);
    if (mysql_query(conn, buf))
    {
        exit(-1);
    }
    

    printf("注册成功\n");

    return 0;
        
    
    
}

//判断账号密码是否正确   正确返回0，错误返回-1
static int determineIfItExists(chatRoomMessage *Message, MYSQL * conn)
{
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "SELECT accountNumber FROM chatRoom WHERE accountNumber = '%s'", Message->accountNumber);
    if (mysql_query(conn, buffer))
    {
        printf("没有该用户\n");
        exit(-1);
    }

    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "SELECT accountNumber FROM chatRoom WHERE accountNumber = '%s' and password = '%s'", Message->accountNumber, Message->password);
    if (mysql_query(conn, buffer))
    {
        printf("账号密码不匹配\n");
        exit(-1);
    }

    return 0;
    
    
}

/*登录*/  /*正确返回0， 错误返回-1*/
int chatRoomLogIn(chatRoomMessage *Message, json_object *obj, Friend *client, MYSQL * conn) /*要将账号，密码的信息传到服务端进行验证是否存在，和密码正确与否，因此要用到json_object*/
{
    int ret = 0;
    struct json_object * accountNumVal = json_object_object_get(obj, "accountNum");
    if (accountNumVal == NULL)
    {
        printf("get accountNumVal error\n");
        exit(-1);
    }

    struct json_object * passwordVal = json_object_object_get(obj, "password");
    if (passwordVal == NULL)
    {
        perror("get passwordVal error\n");
        exit(-1);
    }
    
    Message->accountNumber = (char *)json_object_get_string(accountNumVal);
    Message->password = (char *)json_object_get_string(passwordVal);

    ret = determineIfItExists(Message, conn);
    if (ret == -1)
    {
        return -1;
    }
    friendNode *node = (friendNode *)malloc(sizeof(friendNode));
    memset(node, 0, sizeof(node));
    node->data = Message;

    balanceBinarySearchTreeInsert(client, node);


    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "SELECT accountNumber name FROM %sFriend", Message->name);
            if (mysql_query(conn, buffer))
            {
                printf("查无此人\n");
                exit(-1);
            }
    
    chatRoomMessage *friendMessage = (chatRoomMessage *)malloc(sizeof(chatRoomMessage));
    memset(friendMessage, 0, sizeof(friendMessage));
    
    


    MYSQL_RES *res = (MYSQL_RES *)malloc(sizeof(MYSQL_RES));
    memset(res, 0, sizeof(res));
    // 获取结果集
    mysql_free_result(res);
    res = mysql_use_result(conn);
    if (res != NULL) 
    {
        MYSQL_ROW row;
        memset(row, 0, sizeof(row));
        friendNode *node = (friendNode *)malloc(sizeof(friendNode));
        while ((row = mysql_fetch_row(res)) != NULL) 
        {
            // 遍历结果集并输出数据
            
            snprintf(friendMessage->accountNumber, sizeof(friendMessage->accountNumber), "%s", row[0]);
            snprintf(friendMessage->name, sizeof(friendMessage->name), "%s", row[1]);

                // 处理完一行数据后的其他操作
            
            memset(node, 0, sizeof(node));
            node = (friendNode *)friendMessage;
            balanceBinarySearchTreeInsert(client, node);
        }
        // 释放结果集内存
        mysql_free_result(res);  // 释放查询结果集
        
    }
    return 0;
    
}




/*添加好友*/
int chatRoomAppend(chatRoomMessage *Message, json_object *obj, MYSQL * conn, Friend *Info, Friend *client) /*查找到提示是否要添加该好友，当点了是时，被添加的客户端接收到是否接受该好友，点否则添加不上，发给他一个添加失败，点接受，则将好友插入到你的数据库表中，同时放入以自己的树中*/
{
    printf("请选择 1.昵称查找 2.用账号查找\n");
    int flag = 0;
    
    chatRoomMessage *friendMessage =(chatRoomMessage *)malloc(sizeof(chatRoomMessage));
    memset(friendMessage, 0, sizeof(friendMessage));
    
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));


    snprintf(buffer, sizeof(buffer), "CREATE TABLE IF NOT EXIST %sFriend (accountNumber char[10] PRIMARY KEY, name text NOT NULL)", friendMessage->name);
    if (mysql_query(conn, buffer))
    {
        printf("系统错误，添加好友失败\n");
        exit(-1);
    } 

    while (1)
    {
        scanf("%d", &flag);

        

        if (flag == 1)      //用账号查找  
        {
            scanf("%s", friendMessage->accountNumber);
            

            snprintf(buffer, sizeof(buffer), "SELECT accountNumber name FROM chatRoom WHERE accountNumber = '%s'", friendMessage->accountNumber);
            if (mysql_query(conn, buffer))
            {
                printf("查无此人\n");    
                exit(-1);
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
                        //snprintf(Friend.accountNumber, sizeof(Friend.accountNumber), "%s", row[0]);
                        snprintf(friendMessage->name, sizeof(friendMessage->name), "%s", row[1]);

                            // 处理完一行数据后的其他操作
                    }
                    mysql_free_result(res);  // 释放查询结果集
                }
                flag = 0;                   /*这里少东西还，*/
                printf("是否要添加此人为好友:\n1.是   2.否\n");
                scanf("%d", &flag);
                if (flag == 1)
                {
                    //创建好友表   有问题   好友表没有标记出来
                    snprintf(buffer, sizeof(buffer), "INSERT INTO %sFriend(accountNumber name) VALUES ('%s', '%s')", Message->name, friendMessage->accountNumber, friendMessage->name);
                    if (mysql_query(conn, buffer))
                    {
                        printf("系统错误，添加好友失败\n");
                        exit(-1);
                    }
                    friendNode *node = (friendNode *)malloc(sizeof(friendNode));
                    memset(node, 0, sizeof(node));
                    node = (friendNode *)friendMessage;
                    //插入到好友列表
                    balanceBinarySearchTreeInsert(client, friendMessage);

                }    
                else if (flag == 2)
                {
                    printf("返回成功\n");
                    exit(0);
                }
                else
                {
                    printf("输入内容不符\n");
                    exit(-1);
                }
            }
        }
        else if (flag == 2)     //用昵称查找
        {
            scanf("%s", friendMessage->name);
            char buffer[BUFFER_SIZE];
            memset(buffer, 0, sizeof(buffer));

            snprintf(buffer, sizeof(buffer), "SELECT accountNumber name FROM chatRoom WHERE name = '%s'", Message->name);
            if (mysql_query(conn, buffer))
            {
                printf("查无此人\n");
                exit(-1);
            }
            else
            {
                MYSQL_RES *res = mysql_use_result(conn);
                if (res != NULL) 
                {
                    MYSQL_ROW row;
                    if ((row = mysql_fetch_row(res)) != NULL) 
                    {
                        snprintf(friendMessage->accountNumber, sizeof(friendMessage->accountNumber), "%s", row[0]);
                        // snprintf(Friend->name, sizeof(Friend->name), "%s", row[1]);

                            // 处理完一行数据后的其他操作
                    }
                    mysql_free_result(res);  // 释放查询结果集
                }

                flag = 0;                   /*这里少东西还，*/
                printf("是否要添加此人为好友:\n1.是   2.否\n");
                scanf("%d", &flag);
                if (flag == 1)
                {
                    //创建好友表   有问题   好友表没有标记出来
                    snprintf(buffer, sizeof(buffer), "INSERT INTO %sFriend(accountNumber name) VALUES ('%s', '%s')", Message->name, friendMessage->accountNumber, friendMessage->name);
                    if (mysql_query(conn, buffer))
                    {
                        printf("系统错误，添加好友失败\n");
                        exit(-1);
                    }
                    friendNode *node = (friendNode *)malloc(sizeof(friendNode));
                    memset(node, 0, sizeof(node));
                    node = (friendNode *)friendMessage;
                    //插入到好友列表
                    balanceBinarySearchTreeInsert(client, friendMessage);
                    //添加好友到树中
                    balanceBinarySearchTreeInsert(client, friendMessage);
                }    
                else if (flag == 2)
                {
                    printf("返回成功\n");
                    exit(0);
                }
                else
                {
                    printf("输入内容不符\n");
                    exit(-1);
                }
            }
        }
        else
        {
            printf("输入有误，请重新输入\n");
            exit(-1);
        }
    }
    
    return 0;

}

/* 在线列表的插入 */
int chatRoomOnlineTable(chatRoomMessage *Message, int sockfd, HashTable *onlineTable)
{
    int ret = 0;
    hashTableInsert(onlineTable, Message->name, sockfd);
    return ret;
}

/*输入好友名字 判断好友是否在线*/
int serchFriendIfOnline(HashTable * online, char * name)
{
    int sockfd_onlineFriend = 0;
    if (online == NULL)
    {
        printf("无在线好友\n");
        return 0;
    }
    if (name == NULL)
    {
        prinrf("好友名输入错误\n");
        return 0;
    }
    int * mapVal = NULL;
    if (! hashTableGetAppointKeyValue(online, name, mapVal))   /*找到在线好友的句柄*/ 
    {
        sockfd_onlineFriend = (int)mapVal;
        return sockfd_onlineFriend;
    } 
    return -1;
}

/* 指定好友是否在线 */
int FriendOnlineOrNot(Friend *client, HashTable *onlineTable, chatRoomMessage *Message, int sockfd)
{
    int ret = 1;
    if(balanceBinarySearchTreeIsContainAppointVal(client, Message))
    {
        if(chatRoomOnlineTable(Message, sockfd, onlineTable) == 0)
        {
            printf("在线");
        }
        else
        {
            printf("离线");
            return -1;
        }
    }
    else
    {
        printf("没有此好友！");
        return 0;
    }
    return ret;
}

/*建立私聊的联系*/
int chatRoomPrivateChat(chatRoomMessage *Message, json_object *obj) /*建立一个联系只有双方能够聊天*/ /*判断其书否在线， 是否存在这个好友*/
{ 
    
}

/*建立一个群聊的联系，建立完后将其存储起来*/
int chatRoomGroupChat(chatRoomMessage *Message, json_object *obj) /*通过UDP进行群发，一些人能够接到*/ /*有点问题后面再想*/
{

}

/*删除好友的销毁信息*/
int chatRoomDestroy(chatRoomMessage *Message, json_object *obj, Friend * Info, MYSQL * conn) /*通过传进来的信息，把数据库中你的好友表中的指定人员信息删除，同时删掉内存中的该信息，释放该内存*/
{
    Friend * data = Info;
    int flag = 0;

    chatRoomMessage *friendMessage = (chatRoomMessage *)malloc(sizeof(chatRoomMessage));
    printf("请输入要删除好友的姓名\n");
    scanf("%s", friendMessage->name);
    
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "SELECT name FROM %sFriend WHERE name = %s", Message->name, friendMessage->name);
    if (mysql_query(conn, buffer))
    {
        printf("没有该好友\n");
        exit(-1);
    }
    printf("是否要删除该好友 1.是\t2.否\n");
    scanf("%d", &flag);
    
    if (flag == 1)
    {
        snprintf(buffer, sizeof(buffer), "DELETE FROM %sFriend WHERE name = %s", Message->name, friendMessage->name);
        if (mysql_query(conn, buffer))
        {
            printf("系统错误,删除失败\n");
            exit(-1);
        }
        else 
        {   
            
            if (balanceBinarySearchTreeDelete(Info, friendMessage->name) != 0)
            {
                exit(-1);
            }
            printf("删除成功\n");
            exit(0);
        }
    }
    else if (flag == 2)
    {
        printf("取消成功\n");
        exit(0);
    }
    else
    {
        printf("输入内容不符\n");
        return -1;
    }

    return 0;
    
}

/*注销账号*/
int chatRoomMessageLogOff(chatRoomMessage *Message, json_object *obj) /*通过你的账号信息，删除数据库中用户表中你的信息， 因为该表为主表要先删除附表中他的信息，删除完毕后释放通信句柄，退出到主页面*/
{
    
}

/*输入地址的静态*/
static int inputPath (char * path)
{

    scanf("%s", path);
    
    int exit_ret = 0;
    int choice = 0;
    exit_ret = fileEixt(path);
    while (exit_ret == -1)    /*文件不存在*/
    {
        printf("输入的文件路径不对或者文件不存在,请选择: 1.重新输入 2.退出\n");
        scanf("%d", &choice);
        switch (choice)
        {
        case ONE:   printf("请重新输入文件地址\n");
                    scanf("%s", path);
                    exit_ret = fileEixt(path);
                    break;
        case Two:   exit_ret = 2;    /*退出*/ 
                    break;
        default:
                    printf("无效的选择，请重新输入\n");
                    break;
        }        
        system("clear");
    }
    /*程序执行到这里有两种情况：1、exit_ret = 2退出 2、exit_ret = 1输入的文件名正确*/
    return exit_ret;
}

/*判断输入的路径的文件是否存在*/
static int fileEixt(const char * filePath)
{
    if (filePath == NULL)
    {
        return PATH_ERR;
    }
    if (access (filePath, F_OK) == 0)   /*文件存在且有对应的权限*/
    {
        return FILE_EXIT;
    }
    /*文件存在返回1 不存在返回-1*/
    return PATH_ERR;
}

/*文件传输*/                                                         /*后面再加*/
int chatRoomFileTransfer(chatRoomMessage *Message, json_object *obj) /*通过账号信息找到要发送的人，再通过操作将文件发送过去， 接收到提示要不要接受该文件*/
{
    int ret = 0;
    int choice = 0;
    char file_path[FILE_PATH];
    memset(file_path, 0, FILE_PATH);
    struct stat fileStat;
    while(ret == 0)
    {
        printf("请选择1、输入你想要发送的文件地址 2、退出返回上一个界面\n");
        scanf("%d", &choice);
        switch (choice)
        {
            case ONE:   ret = inputPath(file_path);  /*两种返回值 1、exit_ret = 2退出 2、exit_ret = 1输入的文件名正确*/
                        break;
            case Two:   ret = 2;
                        break;
            default:
                        printf("无效的选择，请重新输入\n");
                        ret = 0;
                        break;
        }        
    }
    /*程序执行到这里有两种情况：1、ret = 2退出 2、ret = 1输入的文件名正确*/
    if (ret == 1)   /*输入的文件名正确*/
    {
        /*打开文件*/
        FILE *fp = fopen(file_path, "rb");
        if(fp == NULL) 
        {
            return FILE_EXIT;
        }
        /*获取文件信息*/
        if (stat(file_path, &fileStat) == -1) 
        {
            printf("无法获取文件信息\n");
            return 0;   /*退出*/
        }
        json_object_object_add(obj,"name" , json_object_new_string(file_path));
        json_object_object_add(obj, "size", json_object_new_int64(fileStat.st_size));
        fclose(fp);
    }
    else if (ret == 2)  /*退出*/
    {
        return 0;
    }
    return 0;
}

/*将Message转换成json格式的字符串进行传送*/
int chatRoomObjConvert(char * buffer, chatRoomMessage * Message, json_object * obj) 
{

     // 创建 json 对象并添加字段
    if (json_object_object_add(obj, "accountNumber", json_object_new_string(Message->accountNumber)) != 0) 
    {
        fprintf(stderr, "json_object_object_add failed for accountNumber\n");
        return -1;
    }

    if (json_object_object_add(obj, "password", json_object_new_string(Message->password)) != 0) 
    {
        fprintf(stderr, "json_object_object_add failed for password\n");
        return -1;
    }

    if (json_object_object_add(obj, "name", json_object_new_string(Message->name)) != 0) 
    {
        fprintf(stderr, "json_object_object_add failed for name\n");
        return -1;
    }

    if (json_object_object_add(obj, "mail", json_object_new_string(Message->mail)) != 0) 
    {
        fprintf(stderr, "json_object_object_add failed for mail\n");
        return -1;
    }

    // 将 json 对象转换为字符串，并拷贝到 buffer 中
    const char * json_str = json_object_to_json_string(obj);
    strncpy(buffer, json_str, BUFFER_SIZE - 1);
    buffer[BUFFER_SIZE - 1] = '\0';

    // 释放分配的内存
    json_object_put(obj);
    return 0;
}


/*将json格式的字符串转换成原来Message*/
int chatRoomObjAnalyze(char * buffer, chatRoomMessage * Message, json_object * obj)
{
    // 将 json 格式的字符串转换为 json 对象
    obj = json_tokener_parse(buffer);
    // printf("________OBJ:%p\n", obj);
    if (obj == NULL) 
    {
        fprintf(stderr, "json_tokener_parse failed\n");
        return -1;
    }

    // 从 json 对象中读取字段
    struct json_object * accountNumberObj = json_object_object_get(obj, "accountNumber");
    if (accountNumberObj != NULL) 
    {
        const char * accountNumber = json_object_get_string(accountNumberObj);
        strncpy(Message->accountNumber, accountNumber, sizeof(Message->accountNumber) - 1);
        Message->accountNumber[sizeof(Message->accountNumber) - 1] = '\0';
    }

    struct json_object * passwordObj = json_object_object_get(obj, "password");
    if (passwordObj != NULL) 
    {
        const char * password = json_object_get_string(passwordObj);
        strncpy(Message->password, password, sizeof(Message->password) - 1);
        Message->password[sizeof(Message->password) - 1] = '\0';
    }

    struct json_object * nameObj = json_object_object_get(obj, "name");
    if (nameObj != NULL) 
    {
        const char * name = json_object_get_string(nameObj);
        strncpy(Message->name, name, sizeof(Message->name) - 1);
        Message->name[sizeof(Message->name) - 1] = '\0';
    }

    struct json_object * mailObj = json_object_object_get(obj, "mail");
    if (mailObj != NULL) 
    {
        const char * mail = json_object_get_string(mailObj);
        strncpy(Message->mail, mail, sizeof(Message->mail) - 1);
        Message->mail[sizeof(Message->mail) - 1] = '\0';
    }

    // 释放 json 对象的内存
    if (obj != NULL) 
    {
        json_object_put(obj);
    }
    return 0;
}

/*将客户端的信息传入json*/ 
int chatRoomClientMeassage(char * buffer, chatRoomMessage * Message, json_object * obj) 
{
    printf("请输入账号\n");
    scanf("%s", Message->accountNumber);
    printf("请输入密码\n");
    scanf("%s", Message->password);
    printf("请输入昵称\n");
    scanf("%s", Message->name);
    printf("请输入邮箱\n");
    scanf("%s", Message->mail);

    chatRoomObjConvert(buffer, Message, obj);
    /*将输入的字符转成json型的字符串*/

    return 0;
}

int chatRoomOnlineInformation(int sockfd, char *buffer, chatRoomMessage * Message, Friend * online, json_object * obj)/*在服务器中用来存放在线人员的昵称以及通信句柄是树结构 通信时会用到*/
{
    chatRoomObjAnalyze(buffer, Message, obj);

    // struct json_object * onlineObj = (json_object *)malloc(sizeof(json_object));
    // memset(onlineObj, 0, sizeof(onlineObj));


    // struct json_object * nameObj = json_object_new_string(Message->name);
    // json_object_object_add(onlineObj, "name", nameObj);

    // struct json_object * sockfdObj = json_object_new_int64(sockfd);
    // json_object_object_add(onlineObj, "fd", sockfdObj);


    // /*这里有问题如何唯一标识每一个用户在树中的位置*/
    // // char *onlineBuf = (char *)json_object_get_string(obj);
    // balanceBinarySearchTreeInsert(online, onlineObj);

    return 0;
}

// int chatRoomOnlineConversion(json_object * onlineObj, Friend * online, int *sockfd)/*在服务其中找到相应的人员通过*/
// {
//     struct json_object *obj = (json_object *)malloc(sizeof(json_object));
//     memset(obj, 0, sizeof(obj));

//     balanceBinarySearchTreeIsContainAppointVal(online, )
// }

