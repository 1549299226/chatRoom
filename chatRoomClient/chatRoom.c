#include <stdio.h>
#include "chatRoom.h"
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <mysql/mysql.h>
#include <sys/stat.h>
#include <json-c/json_object.h>
#include <sys/types.h>
#include <sys/socket.h>

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

#define CONTENT_MAX 128

#define BUFFER_SIZE 100
#define SEND_BUFFER 140

#define CONTEBNT_SIZE 1024
// struct 
// {
//     /* data */
// }


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

int existenceOrNot(void *arg1, void *arg2)
{
    chatContent *idx1 = (chatContent *) arg1;
    chatContent *idx2 = (chatContent *) arg2;
    // char * idx1 = (char *)arg1;
    // char * idx2 = (char *)arg2;
    int result = 0;
    result = strcmp(idx1->friendName, idx2->friendName);

    return result;
}

static int fileEixt(const char * filePath);

static int accountRegistration(char * accountNumber , MYSQL * conn);    //判断账号是否合法

static int registrationPassword(char * password);       //判断密码是否合法

static int nameLegitimacy(char * name, MYSQL * conn);  //判断昵称的合法性

static int determineIfItExists(chatRoomMessage *Message, MYSQL * conn); //判断账号密码是否正确


/*初始化聊天室*/
int chatRoomInit(chatRoomMessage **Message, groupChat ** groupChatInfo, chatContent **friendMessage, json_object **obj, Friend *Info, Friend *client, Friend * online, MYSQL ** conn, int (*compareFunc)(ELEMENTTYPE val1, ELEMENTTYPE val2), int (*printFunc)(ELEMENTTYPE val), friendNode *node) /*先这些后面再加*/
{
    int ret = 0;

    (*Message) = (chatRoomMessage *)malloc(sizeof(chatRoomMessage));
    memset((*Message), 0, sizeof(Message));
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

    (*friendMessage) = (chatContent *)malloc(sizeof(chatContent));
    memset((*friendMessage), 0, sizeof(friendMessage));
    /*初始化好友姓名*/
    (*friendMessage)->friendName = (char *)malloc(sizeof(char) * NAMESIZE);
    if ((*friendMessage)->friendName == NULL)
    {
        return MALLOC_ERROR;
    }
    /*清楚脏数据*/
    bzero((*friendMessage)->friendName, sizeof(char) * NAMESIZE);

    /*初始化自己姓名*/
    (*friendMessage)->myName = (char *)malloc(sizeof(char) * NAMESIZE);
    if ((*friendMessage)->myName == NULL)
    {
        return MALLOC_ERROR;
    }
    bzero((*friendMessage)->myName, sizeof(char) * NAMESIZE);

    /*聊天内容初始化*/
    (*friendMessage)->content = (char *)malloc(sizeof(char) * CONTENT_MAX);
    if ((*friendMessage)->content == NULL)
    {
        return MALLOC_ERROR;
    }
    bzero((*friendMessage)->content, sizeof(char) * CONTENT_MAX);

    /*聊天时间初始化*/
    time(&(*friendMessage)->chatTime);

    //群聊结构体初始化
    (*groupChatInfo) = (groupChat *) malloc(sizeof(groupChat));
    memset((*groupChatInfo), 0, sizeof(groupChatInfo));
    //初始化群名
    (*groupChatInfo)->groupChatName = (char *)malloc(sizeof(char) * NAMESIZE);
    if ((*groupChatInfo)->groupChatName == NULL)
    {
        return MALLOC_ERROR;
    }
    bzero((*groupChatInfo)->groupChatName , sizeof(char) * NAMESIZE);
    //初始化群成员名字
    (*groupChatInfo)->membersName = (char *)malloc( sizeof(char) * NAMESIZE);
    if ((*groupChatInfo)->membersName == NULL)
    {
        return MALLOC_ERROR;
    }
    bzero((*groupChatInfo)->membersName, sizeof(char) * NAMESIZE);
    //初始化群聊内容
    (*groupChatInfo)->groupChatContent = (char*)malloc(sizeof(char) * CONTENT_MAX);
    if ((*groupChatInfo)->groupChatContent == NULL)
    {
        return MALLOC_ERROR;
    }
    bzero((*groupChatInfo)->groupChatContent , sizeof(char )* CONTENT_MAX);
    //初始化聊天时间
    time(&(*groupChatInfo)->groupChatTime);

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
int chatRoomInsert(char * buffer, chatRoomMessage *Message, json_object *obj, MYSQL * conn) /*账号不能跟数据库中的有重复，昵称也是不可重复，通过账号算出一个key（用一个静态函数来计算），这个key便是ID是唯一的，密码要包含大写及特殊字符，最少八位，不然密码不符合条件，将注册好的信息放到数据库中*/
{
    
    int ret = 0;
    chatRoomObjAnalyze(buffer, Message, obj);
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
        while ((row = mysql_fetch_row(res)) != NULL) 
        {
            // 遍历结果集并输出数据
            
            snprintf(friendMessage->accountNumber, sizeof(friendMessage->accountNumber), "%s", row[0]);
            snprintf(friendMessage->name, sizeof(friendMessage->name), "%s", row[1]);

                // 处理完一行数据后的其他操作
            
            
            balanceBinarySearchTreeInsert(client, friendMessage);
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

/*获取指定好友的位置*/
static void * baseAppointValGetaddressBookNode(Friend *friendInfo, ELEMENTTYPE data)
{
    friendNode * travelNode = friendInfo->root;
    int cmp = 0;
    while (travelNode != NULL)
    {
        cmp = friendInfo->compareFunc(data, travelNode->data);
        if (cmp < 0)
        {
            travelNode = travelNode->left;
        }
        else if (cmp > 0)
        {
            travelNode = travelNode->right;
        }
        else
        {
            /* 找到 */
            return travelNode->data;
        }
    }
    return NULL;
}

/*输入名字判断好友是否存在*/
int friendIsExit(Friend *Info, ELEMENTTYPE data, char * name)
{
    if (Info == NULL)
    {
        printf("您还没有好友，退出");
        return -1;
    }
    chatContent * info = data;
    int sockOnlinefd = 0;

    char * flag = (char *)malloc(sizeof(char));
    memset(flag, 0, sizeof(flag));

    while (1)
    {
        printf("请输入你要查找的好友名字:\n");
        scanf("%s", info->friendName);
        info = (chatContent *) baseAppointValGetaddressBookNode(Info, data);
        while(info == NULL)
        {
            printf("查无此人\n");
            printf("请选择1、退出 2、重新输入");
            scanf("%s", flag);
            if (!strncmp(flag, "1", sizeof(flag)))
            {
                return -1;
            }
            else if (!strncmp(flag, "2", sizeof(flag)))
            {
                break;  /*跳出当前while循环*/
            }
            else
            {
                printf("无效的输入，重新输入\n");
                continue;
            }
        }

        
        
    //     if (chatRoomOnlineOrNot != -1)  /* to do...等接口*/
    //     {
    //         sockOnlinefd = chatRoomOnlineOrNot(info, obj);
    //         return sockOnlinefd;
    //     }
    }

    printf("找到好友信息:\n");
    name = info->friendName;
    Info->printFunc(info);
        
    free (flag);
    flag = NULL;
    return 1;
}



/*看是否有人在线*/
int chatRoomOnlineOrNot(chatRoomMessage *Message, json_object *obj) /*每过一段时间向各个客户发一个消息，如果能发出去，判其为在线状态，返回0，不在线则返回0*/
{
    
}

static char *createJSONMessage(chatContent *chat);
// 创建一个函数来将 chatContent 结构体转换为 JSON 字符串
static char *createJSONMessage(chatContent *chat)
{
    struct json_object *jobj = json_object_new_object();
    
    json_object_object_add(jobj, "friendName", json_object_new_string(chat->friendName));
    json_object_object_add(jobj, "myName", json_object_new_string(chat->myName));
    json_object_object_add(jobj, "content", json_object_new_string(chat->content));
    json_object_object_add(jobj, "chatTime", json_object_new_int(chat->chatTime));

    const char *jsonStr = json_object_to_json_string(jobj);

    char *result = strdup(jsonStr);

    json_object_put(jobj); // 释放 jobj 对象
    free((void *)jsonStr); // 释放 jsonStr
    free(result);
    return result;
}

static void truncateString(char *str);
/*判断发送的消息是否超过140字符*/
static void truncateString(char *str)
{
    int len = strlen(str);
    if (len > SEND_BUFFER) 
    {
        printf("发送的消息超过140字符,只保留140字符\n");
        str[SEND_BUFFER - 1] = '\0';
        
    }
}


/*建立私聊的联系*/
int chatRoomPrivateChat( char * friendName, int sockfd, chatContent * chat, chatRoomMessage * message) 
/*建立一个联系只有双方能够聊天*/ /*判断其书否在线， 是否存在这个好友*/
{ 
    while (1)
    {
        printf("请选择1、发消息 2、发文件 3、退出\n");
        char * flag = (char *)malloc(sizeof(char));
        memset(flag, 0, sizeof(flag));

        char sendBuffer[SEND_BUFFER];
        memset(sendBuffer, 0, sizeof(sendBuffer));
        scanf("%s", flag);

        int ret = 0;
        while (1)
        {
            if (!strncmp(flag, "1", sizeof(flag)))
            {
                printf("1、请输入你要发送的消息(不超过140字符): 2、退出返回上一级\n");
                if (!strncmp(flag, "1", sizeof(flag)))  /*输入发送的消息*/
                {
                    memset(chat->content, 0, sizeof(chat->content));
                    scanf("%s", chat->content);   /*输入要发送的内容*/
                    if(getchar() == '\n')
                    {
                        printf("输入的内容为空，请重新输入\n");
                        continue;
                    }
                    /*判断发送的消息是否超过140字符*/
                    truncateString(chat->content);
                    
                    chat->chatTime = time(NULL);
                    chat->friendName = friendName;
                    chat->myName = message->name;

                    char * json_chat = createJSONMessage(chat);

                    strncpy(sendBuffer, json_chat, sizeof(sendBuffer));
                    memset(sendBuffer, 0, sizeof(sendBuffer));
                    ret = send(sockfd, sendBuffer, sizeof(sendBuffer), 0);
                    if (ret < 0)
                    {
                        printf("发送失败：%s\n", strerror(errno));
                        printf("返回上一级\n");
                        continue;
                    }
                    else if (!strncmp(flag, "2", sizeof(flag))) 
                    {
                        break;
                    }
                    else
                    {
                        printf("无效的输入，返回上一级\n");
                        continue;
                    }
                    free(json_chat);
                }

                
            }
            else if(!strncmp(flag, "2", sizeof(flag)))
            {
                /*发文件 to do..*/
                printf("该功能尚未完善，返回上一级\n");
                continue;
            }
            else if (!strncmp(flag, "3", sizeof(flag)))
            {
                return -1;
            }
        }
        free(flag);
        }
    
    return 0;
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
    /*文件存在返回1 存在返回-1*/
    return PATH_ERR;
}

/*输入地址的静态*/
static int inputPath(char * path)
{

    scanf("%s", path);
    int exit_ret = 0;
    int choice = 0;
    exit_ret = fileEixt(path);
    while (exit_ret == -1)    /*文件不存在*/
    {
        printf("输入的文件路径不对或者文件不存在,请选择: 1.重新输入 2.退出\n");
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

/*文件传输*/                                                         /*后面再加*/
int chatRoomFileTransfer(chatRoomMessage *Message, json_object *obj) /*通过账号信息找到要发送的人，再通过操作将文件发送过去， 接收到提示要不要接受该文件*/
{
    int ret = 0;
    int choice = 0;
    char * file_path = NULL;
    struct stat fileStat;
    while(ret == 0)
    {
        printf("请选择1、输入你想要发送的文件地址 2、退出返回上一个界面\n");
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
        /*获取文件信息*/
        if (stat(file_path, &fileStat) == -1) 
        {
            printf("无法获取文件信息\n");
            
        }
        json_object_object_add(obj,"name" , json_object_new_string(file_path));
        json_object_object_add(obj, "size", json_object_new_int64(fileStat.st_size));
        
    }
    else if (ret == 2)  /*退出*/
    {
        return 0;
    }
    
}

/*将Message转换成json格式的字符串进行传送*/
int chatRoomObjConvert(char * buffer, chatRoomMessage * Message, json_object * obj) 
{
    obj = json_object_new_object();
    //  创建 json 对象并添加字段
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

/*将客户端的信息传入json*/ 
int chatRoomClientLogIn(char * buffer, chatRoomMessage * Message, json_object * obj) 
{
    printf("请输入账号\n");
    scanf("%s", Message->accountNumber);
    printf("请输入密码\n");
    scanf("%s", Message->password);
    
    

    chatRoomObjConvert(buffer, Message, obj);
    /*将输入的字符转成json型的字符串*/

    return 0;
}

/*将chatContent转换成json格式的字符串进行传送*/
int chatRoomObjConvertContent(char * buffer, chatContent * chat, json_object * obj) 
{

    obj = json_object_new_object();
     // 创建 json 对象并添加字段
    if (json_object_object_add(obj, "friendName", json_object_new_string(chat->friendName)) != 0) 
    {
        fprintf(stderr, "json_object_object_add failed for friendName\n");
        return -1;
    }

    if (json_object_object_add(obj, "myName", json_object_new_string(chat->myName)) != 0) 
    {
        fprintf(stderr, "json_object_object_add failed for myName\n");
        return -1;
    }

    if (json_object_object_add(obj, "content", json_object_new_string(chat->content)) != 0) 
    {
        fprintf(stderr, "json_object_object_add failed for content\n");
        return -1;
    }

    if (json_object_object_add(obj, "time", json_object_new_int64(chat->chatTime)) != 0) 
    {
        fprintf(stderr, "json_object_object_add failed for time\n");
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
    
/*将json格式的字符串转换成原来chat*/
int chatRoomObjAnalyzeContent(char * buffer, chatContent * chat, json_object * obj)
{
    // 将 json 格式的字符串转换为 json 对象
    obj = json_object_new_object();
    obj = json_tokener_parse(buffer);
    if (obj == NULL) 
    {
        fprintf(stderr, "json_tokener_parse failed\n");
        return -1;
    }
    
    // 从 json 对象中读取字段
    struct json_object * myNameObj = json_object_object_get(obj, "myName");
    if (myNameObj != NULL) 
    {
        const char * myName = json_object_get_string(myNameObj);
        strncpy(chat->myName, myName, sizeof(chat->myName) - 1);
        chat->myName[sizeof(chat->myName) - 1] = '\0';
    }

    struct json_object * friendNameObj = json_object_object_get(obj, "friendName");
    if (friendNameObj != NULL) 
    {
        const char * friendName = json_object_get_string(friendNameObj);
        strncpy(chat->friendName, friendName, sizeof(chat->friendName) - 1);
        chat->friendName[sizeof(chat->friendName) - 1] = '\0';
    }

    struct json_object * contentObj = json_object_object_get(obj, "content");
    if (contentObj != NULL) 
    {
        const char * content = json_object_get_string(contentObj);
        strncpy(chat->content, content, CONTEBNT_SIZE - 1);
        chat->content[CONTEBNT_SIZE - 1] = '\0';
    }

    struct json_object * timeObj = json_object_object_get(obj, "time");
    if (timeObj != NULL) 
    {
        chat->chatTime = json_object_get_int64(timeObj);
    }

    // 释放 json 对象的内存
    if (obj != NULL) 
    {
        json_object_put(obj);
    }
    return 0;
}

// 释放聊天室消息结构体
static void freeChatRoomMessage(chatRoomMessage *Message) ;
// 释放聊天室消息结构体
static void freeChatRoomMessage(chatRoomMessage *Message) 
{
    if (Message != NULL) {
        free(Message->name);
        free(Message->accountNumber);
        free(Message->mail);
        free(Message->password);
        free(Message);
        // Message->name = NULL;
        // Message->accountNumber = NULL;
        // Message->mail = NULL;
        // Message->password = NULL;
        // Message = NULL;


    }
}

// 释放聊天内容结构体
static void freeChatContent(chatContent *content) ;

static void freeChatContent(chatContent *content) 
{
    if (content != NULL) 
    {
        free(content->friendName);
        free(content->myName);
        free(content->content);
        free(content);
        // content->friendName = NULL;
        // content->myName = NULL;
        // content->content = NULL;
        // content = NULL;


    }
}

// 释放群聊结构体
static void freeGroupChat(groupChat *group) ;
static void freeGroupChat(groupChat *group) 
{
    if (group != NULL) 
    {
        free(group->groupChatName);
        free(group->membersName);
        free(group->groupChatContent);
        free(group);
        // group->groupChatName = NULL;
        // group->membersName = NULL;
        // group->groupChatContent = NULL;
        // group = NULL;
    }
}


// 释放JSON对象
static void freeJsonObj(json_object *obj);
static void freeJsonObj(json_object *obj) 
{
    if (obj != NULL) 
    {
        json_object_put(obj);
        free(obj);
       // obj = NULL;
    }
}

// 释放好友列表结点
static void freeFriendNode(friendNode *node) ;
static void freeFriendNode(friendNode *node) 
{
    if (node != NULL) 
    {
        freeChatRoomMessage(node->data);
        free(node);
        //node = NULL;
    }
}

// 释放好友列表
static void freeFriendList(Friend *list); 

static void freeFriendList(Friend *list) 
{
    if (list != NULL) 
    {
        balanceBinarySearchTreeDestroy(list);
    }
}

// 关闭数据库连接
static void closeDBConnection(MYSQL *conn) ;

static void closeDBConnection(MYSQL *conn) 
{
    if (conn != NULL) 
    {
        mysql_close(conn);
        //conn = NULL;
    }
}

/* 退出登录时的资源回收 */
void logoutCleanup(Friend * client, Friend* online, Friend * Info, chatRoomMessage *Message, chatContent *friendMessage, json_object *obj, MYSQL *conn, friendNode *node, groupChat * groupChatInfo) 
{
   
    // 释放好友列表
    freeFriendList(client);
    // 释放好友列表结点
    //freeFriendNode(node);
    // 释放JSON对象
    //freeJsonObj(obj);
    // obj = NULL;
    // 释放群聊结构体
    freeGroupChat(groupChatInfo);
    // 释放聊天内容结构体
    freeChatContent(friendMessage);
    // 释放聊天室消息结构体
    freeChatRoomMessage(Message);
    freeFriendList(online);
    freeFriendList(Info);
    // 关闭数据库连接
    //closeDBConnection(conn);

}

/*将json格式的字符串转换成原来onlineHash*/
int chatHashObjAnalyze(char * buffer, chatHash * onlineHash, json_object * obj)
{
    // 将 json 格式的字符串转换为 json 对象
    obj = json_tokener_parse(buffer);
    if (obj == NULL) 
    {
        fprintf(stderr, "json_tokener_parse failed\n");
        return -1;
    }
    
    // 从 json 对象中读取字段
    struct json_object * hashNameObj = json_object_object_get(obj, "hashName");
    if (hashNameObj != NULL) 
    {
        const char * hashName = json_object_get_string(hashNameObj);
        strncpy(onlineHash->hashName, hashName, sizeof(onlineHash->hashName) - 1);
        onlineHash->hashName[sizeof(onlineHash->hashName) - 1] = '\0';
    }

    struct json_object * sockfdObj = json_object_object_get(obj, "sockfd");
    if (sockfdObj != NULL) 
    {
        int64_t sockfd = json_object_get_int64(sockfdObj);
        
    }

    // 释放 json 对象的内存
    if (obj != NULL) 
    {
        json_object_put(obj);
    }
    return 0;
}


/*将hash转换成json格式的字符串进行传送*/
int chatHashObjConvert(char * buffer, chatHash * onlineHash, json_object * obj) 
{
    

    printf("1356 %s---%d\n", onlineHash->hashName, onlineHash->sockfd);
    obj = json_object_new_object();
    
    if (!json_object_is_type(obj, json_type_object)) {
        fprintf(stderr, "obj is not an object\n");
        sleep(2);
        return -1;
    }
    //  创建 json 对象并添加字段
    if (json_object_object_add(obj, "hashName", json_object_new_string((const char *)onlineHash->hashName)) != 0) 
    {
        fprintf(stderr, "json_object_object_add failed for hashName\n");
        return -1;
    }

    if (json_object_object_add(obj, "sockfd", json_object_new_int64(onlineHash->sockfd)) != 0) 
    {
        fprintf(stderr, "json_object_object_add failed for sockfd\n");
        return -1;
    }
    

    // 将 json 对象转换为字符串，并拷贝到 buffer 中
    const char * json_str = json_object_to_json_string(obj);
    strncpy(buffer, json_str, BUFFER_SIZE - 1);
    buffer[BUFFER_SIZE - 1] = '\0';
    //printf("%s\n", buffer);
    printf("1374 --%s\n", buffer);
    // 释放分配的内存
    json_object_put(obj);
    return 0;
}


/*将回调中的Message转换成json格式的字符串进行传送*/
int printStructObj(char * buffer, chatRoomMessage * Message, json_object * obj) 
{

    obj = json_object_new_object();
     // 创建 json 对象并添加字段
    if (json_object_object_add(obj, "accountNumber", json_object_new_string(Message->accountNumber)) != 0) 
    {
        fprintf(stderr, "json_object_object_add failed for accountNumber\n");
        return -1;
    }

    if (json_object_object_add(obj, "name", json_object_new_string(Message->name)) != 0) 
    {
        fprintf(stderr, "json_object_object_add failed for name\n");
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
int objPrintStruct(char * buffer, chatRoomMessage * Message, json_object * obj)
{
    // 将 json 格式的字符串转换为 json 对象
    obj = json_tokener_parse(buffer);
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

    struct json_object * nameObj = json_object_object_get(obj, "name");
    if (nameObj != NULL) 
    {
        const char * name = json_object_get_string(nameObj);
        strncpy(Message->name, name, sizeof(Message->name) - 1);
        Message->name[sizeof(Message->name) - 1] = '\0';
    }

    // 释放 json 对象的内存
    if (obj != NULL) 
    {
        json_object_put(obj);
    }
    return 0;
}
