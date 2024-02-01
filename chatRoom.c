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
#define DBHOST "127.0.0.1"
#define DBUSER "root"
#define DBPASS "1"
#define DBNAME "chatRoom"

#define BUFFER_SIZE 100


static int accountRegistration(char * accountNumber , MYSQL * conn);    //判断账号是否合法

static int registrationPassword(char * password);       //判断密码是否合法

static int nameLegitimacy(char * name, MYSQL * conn);  //判断昵称的合法性

static int determineIfItExists(chatRoomMessage *Message, MYSQL * conn); //判断账号密码是否正确


/*初始化聊天室*/
int chatRoomInit(chatRoomMessage *Message, json_object *obj, Friend *Info, Friend *client, MYSQL * conn, int (*compareFunc)(ELEMENTTYPE val1, ELEMENTTYPE val2), int (*printFunc)(ELEMENTTYPE val), friendNode *node) /*先这些后面再加*/
{
    int ret = 0;

    Message = (chatRoomMessage *)malloc(sizeof(chatRoomMessage));
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
    bzero(Message->accountNumber, sizeof(char) * ACCOUNTNUMBER);

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

    balanceBinarySearchTreeInit(&client, compareFunc, printFunc);


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
    conn = mysql_init(NULL);        
    if (conn == NULL)           /*判断是否正确*/
    {
        fprintf(stderr, "mysql_init failed\n");
        return MALLOC_ERROR;
    }

    /*连接数据库*/
    if (mysql_real_connect(conn, DBHOST, DBUSER, DBPASS, NULL, 0, NULL, 0) == NULL) 
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

    char buffer[BUFFER_SIZE << 2];
    memset(buffer, 0, sizeof(buffer));
    //创建表
   snprintf(buffer, sizeof(buffer), "CREATE TABLE IF NOT EXISTS chatRoom "
                                 "(accountNumber VARCHAR(100) PRIMARY KEY, "
                                 "password VARCHAR(100) NOT NULL, "
                                 "name VARCHAR(100) NOT NULL, "
                                 "mail VARCHAR(100) NOT NULL)");
        
    if (mysql_query(conn, buffer))
    {
        exit(-1);
    }
    
    printf("你好\n");

    return ret;
}



//判断账号是否正确,正确返回0，错误返回-1
static int accountRegistration(char * accountNumber , MYSQL * conn)        
{
    int len = 0;               //长度
    int flag = 0;            //标记
    int count = 0;           //计数器

    len = sizeof(accountNumber);

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
    snprintf(buffer, sizeof(buffer), "SELECT accountNumber FROM chatRoom WHERE  accountNumber = '%s'", accountNumber);
    
    /*判断该账号是否在数据库中*/
    if (mysql_query(conn, buffer))
    {
        return 0;
    }

    printf("已有该账号\n");
    return -1;
}

//判断密码是否合法
static int registrationPassword(char * password)     //判断密码是否正确,正确返回0，错误返回-1
{
    int letter = 0;           // 记录是否有字母
    int figure = 0;           // 记录是否有数字
    int specialCharacter = 0; // 记录是否有特殊字符
    int len = sizeof(password);
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

    if (mysql_query(conn, buffer))
    {
        printf("已有该昵称\n");
        memset(name, 0, sizeof(name));
        return -1;
    }
    
    return 0;
}

/*注册*/    //注册成功返回0， 失败返回-1
int chatRoomInsert(chatRoomMessage *Message, json_object *obj, MYSQL * conn) /*账号不能跟数据库中的有重复，昵称也是不可重复，通过账号算出一个key（用一个静态函数来计算），这个key便是ID是唯一的，密码要包含大写及特殊字符，最少八位，不然密码不符合条件，将注册好的信息放到数据库中*/
{
    
    int ret = 0;

    ret = accountRegistration(Message->accountNumber, conn);  /*判断账号是否合法*/
    if (ret == -1)      
    {
        return -1;
    }


    /**/
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
    
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    snprintf(buffer, sizeof(buffer), "INSERT INTO chatRoom VALUES ('%s', '%s', '%s', '%s')", 
                Message->accountNumber, Message->password, Message->name, Message->mail);
    if (mysql_query(conn, buffer))
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

/*文件传输*/                                                         /*后面再加*/
int chatRoomFileTransfer(chatRoomMessage *Message, json_object *obj) /*通过账号信息找到要发送的人，再通过操作将文件发送过去， 接收到提示要不要接受该文件*/
{
    printf("请输入你想要发送的文件地址:\n");
    


}

int chatRoomObjConvert(char * buffer, chatRoomMessage * Message, json_object * obj)
{

    struct json_object * accountNumberObj = json_object_new_string(Message->accountNumber);
    json_object_object_add(obj, "accountNumber", accountNumberObj);

    struct json_object * passwordObj = json_object_new_string(Message->password);
    json_object_object_add(obj, "accountNumber", passwordObj);

    struct json_object * nameObj = json_object_new_string(Message->name);
    json_object_object_add(obj, "accountNumber", nameObj);

    struct json_object * mailObj = json_object_new_string(Message->mail);
    json_object_object_add(obj, "accountNumber", mailObj);

    buffer = json_object_get_string(obj); 
   
    return 0;
}

int chatRoomObjAnalyze(char * buffer, chatRoomMessage * Message, json_object * obj)
{
    obj = json_object_new_string(buffer);
    struct json_object * accountNumberObj = json_object_object_get(obj, "accountNumber");
    Message->accountNumber = json_object_get_string(accountNumberObj);

    struct json_object * passwordObj = json_object_object_get(obj, "password");
    Message->passwordObj = json_object_get_string(passwordObj);

    struct json_object * nameObj = json_object_object_get(obj, "name");
    Message->name = json_object_get_string(nameObj);

    struct json_object * mailObj = json_object_object_get(obj, "mail");
    Message->mail = json_object_get_string(mailObj);

    return 0;
}