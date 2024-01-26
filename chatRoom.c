#include <stdio.h>
#include "chatRoom.h"
#include <stdlib.h>
#include <error.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <mysql/mysql.h>

#define PASSWORD_MAX 8  
#define PASSWORD_MIN 6
#define MAILSIZE 20
#define NAMESIZE 12
#define ACCOUNTNUMBER 6

/*数据库的宏*/
#define DBHOST "localhost"
#define DBUSER "root"
#define DBPASS "1"
#define DBNAME "chatRoom"


static int accountRegistration(char * accountNumber);    //判断账号是否合法

static int registrationPassword(char * password);       //判断密码是否合法


/*初始化聊天室*/
int chatRoomInit(chatRoomMessage *Message, json_object *obj, Friend *Info, MYSQL * conn, int (*compareFunc)(ELEMENTTYPE val1, ELEMENTTYPE val2), int (*printFunc)(ELEMENTTYPE val), friendNode *node) /*先这些后面再加*/
{
    int ret = 0;

    /*初始化姓名*/
    Message->name = (char *)malloc(sizeof(char) * NAMESIZE);
    if (Message->name == NULL)
    {
        return MALLOC_ERROR;
    }
    /*清楚脏数据*/
    bzero(Message->name, sizeof(char) * NAMESIZE);

    /*账号初始化*/
    Message->accountNumber = (char *)malloc(sizeof(char) * ACCOUNTNUMBER);
    if (Message->accountNumber == NULL)
    {
        return MALLOC_ERROR;
    }

    /*邮箱初始化*/
    Message->mail = (char *)malloc(sizeof(char) * MAILSIZE);
    if (Message->mail == NULL)
    {
        return MALLOC_ERROR;
    }
    bzero(Message->mail, sizeof(char) * MAILSIZE);

    /*密码初始化*/
    Message->password = (char *)malloc(sizeof(char) * PASSWORD_MAX);
    if (Message->password == NULL)
    {
        return MALLOC_ERROR;
    }
    bzero(Message->password, PASSWORD_MAX);

    // 创建一个json对象
    obj = json_object_new_object();

    // 将用户列表初始化
    balanceBinarySearchTreeInit(&Info, compareFunc, printFunc);

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
    node->height = 0;
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;

    /*初始化一个数据库*/
    conn = mysql_init(NULL);        
    if (conn == NULL)           /*判断是否正确*/
    {
        fprintf(stderr, "mysql_init failed\n");
        return MALLOC_ERROR;
    }

    /*连接数据库*/
    if (mysql_real_connect(conn, "DBHOST", "DBUSER", "DBPASS", NULL, 0, NULL, 0) == NULL) 
    {
        fprintf(stderr, "mysql_real_connect failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return MALLOC_ERROR;
    }
    
    /*创建数据库*/
    if (mysql_query(conn, "CREATE DATABASE IF NOT EXISTS chatRoom"))
    {
        fprintf(stderr, "Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
    } 
    
    /*打开数据库*/
    if (mysql_select_db(conn, DBNAME)) 
    {
        fprintf(stderr, "Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
        mysql_close(conn);
        exit(1);
    } 


    

    return ret;
}




static int accountRegistration(char * accountNumber)        //判断账号是否正确,正确返回0，错误返回-1
{
    int len = 0;               //长度
    int fang = 0;            //标记
    int count = 0;           //计数器

    len = sizeof(accountNumber);

    while (count < len)                          
    {
              /*保存账号长度*/

        if (count > '9' || count < '0')     /*判断是否满足账号要求*/
        {                                      
            memset(accountNumber, 0, sizeof(accountNumber));   /*不满足条件将内容归零，重新输入*/
            count = 0; 
            fang = 1;
            break;
        }
        count++;
    }   
    if (count != len || fang != 0)   /*判断账号长度是满足条件*/
    {  
        fang += 2;
        if (fang == 2)
        {
            printf("账号长度不符合请重新输入\n");
        }
        else if (fang == 1)
        {
            printf("账号格式不符合请重新输入\n");
        }
        else if (fang == 3)
        {
            printf("账号长度与格式都不符合条件\n");
        }

        sleep(2);
        return -1;
    }

    
    
    return 0;
}

static int registrationPassword(cahr * password)    //判断密码是否合法
{
    int letter = 0;           // 记录是否有字母
    int figure = 0;           // 记录是否有数字
    int specialCharacter = 0; // 记录是否有特殊字符
    int len = sizeof(password);
    int count = 0;
    
    while (count < len)             //判断密码合法否
    {
        if (password[count] > '9' && password[count] < '0')
        {
            
        }
        
    }
        
}

/*注册*/
int chatRoomInsert(chatRoomMessage *Message, json_object *obj) /*账号不能跟数据库中的有重复，昵称也是不可重复，通过账号算出一个key（用一个静态函数来计算），这个key便是ID是唯一的，密码要包含大写及特殊字符，最少八位，不然密码不符合条件，将注册好的信息放到数据库中*/
{
    
    int ret = 0;
    
        printf("请输入账号：(六位0-9的数字)\n");
        scanf("%s", Message->accountNumber);          /*输入账号*/ 

        ret = accountRegistration(Message->accountNumber);  /*判断账号是否合法*/
        if (ret == -1)      
        {
            exit(-1);
        }

        printf("请输入密码：(六到八位，包括大小写，特殊字符，及数字)\n");
        scanf("%s", Message->password);



        printf("请输入你的邮箱\n");
        scanf("%s", Message->mail);

    
    
}

/*登录*/
int chatRoomLogIn(chatRoomMessage *Message, json_object *obj) /*要将账号，密码的信息传到服务端进行验证是否存在，和密码正确与否，因此要用到json_object*/
{
}

/*添加好友*/
int chatRoomAppend(chatRoomMessage *Message, json_object *obj, Friend *Info) /*查找到提示是否要添加该好友，当点了是时，被添加的客户端接收到是否接受该好友，点否则添加不上，发给他一个添加失败，点接受，则将好友插入到你的数据库表中，同时放入以自己的树中*/
{
}

/*看是否有人在线*/
int chatRoomOnlineOrNot(chatRoomMessage *Message, json_object *obj) /*每过一段时间向各个客户发一个消息，如果能发出去，判其为在线状态，返回0，不在线则返回0*/
{
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
int chatRoomDestroy(chatRoomMessage *Message, json_object *obj) /*通过传进来的信息，把数据库中你的好友表中的指定人员信息删除，同时删掉内存中的该信息，释放该内存*/
{
}

/*注销账号*/
int chatRoomMessageLogOff(chatRoomMessage *Message, json_object *obj) /*通过你的账号信息，删除数据库中用户表中你的信息， 因为该表为主表要先删除附表中他的信息，删除完毕后释放通信句柄，退出到主页面*/
{
}

/*文件传输*/                                                         /*后面再加*/
int chatRoomFileTransfer(chatRoomMessage *Message, json_object *obj) /*通过账号信息找到要发送的人，再通过操作将文件发送过去， 接收到提示要不要接受该文件*/
{
}