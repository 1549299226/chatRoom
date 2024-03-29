#include <stdio.h>
#include "chatRoom.h"
#include <stdlib.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <mysql/mysql.h>
#include <sys/stat.h>
#include "hashtable.h"
#include <json-c/json_object.h>
#include <limits.h>

#define PASSWORD_MAX 8  
#define PASSWORD_MIN 6
#define MAILSIZE 20
#define NAMESIZE 12
#define ACCOUNTNUMBER 6
#define FILE_PATH 50


#define CONTENT_MAX 128

/*数据库的宏*/
#define DBHOST "localhost"
#define DBUSER "root"
#define DBPASS "1"
#define DBNAME "chatRoom"


#define BUFFER_SIZE 1024
#define BUFFER_SIZE_M 256
#define MAX_ONLINE 50

#define SLOTNUMS_MAX 200
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

int compareFunc(void *val1, void *val2)
{
   
    hashNode *key1 = (hashNode *)val1;
 

    hashNode *key2 = (hashNode *)val2;
    printf("key1->real_key:%d, key2->real_key:%d\n", key1->real_key, key2->real_key);
    return key1->real_key - key2->real_key;
}

int compareFunc1(void *val1, void *val2)
{
    chatRoomMessage *node1 = (chatRoomMessage *)val1;
    chatRoomMessage *node2 = (chatRoomMessage *)val2;
    printf("node1->name = %s\n", node1->name);
    printf("node2->name = %s\n", node2->name);
    int ret = strncmp(node1->name, node2->name, NAMESIZE);
    return ret;
}

static int fileEixt(const char * filePath);



/*输入地址的静态*/
static int inputPath (char * path);

//获取指定联系人的位置
static void * baseAppointValGetchatRoomSelect(Friend *client, ELEMENTTYPE data);


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
int chatRoomInit(chatRoomMessage ** Message, groupChat ** groupChatInfo, chatContent **friendMessage, json_object **obj, Friend **Info, Friend **client, Friend ** online, MYSQL **conn, int (*compareFunc)(ELEMENTTYPE val1, ELEMENTTYPE val2), int (*compareFunc1)(ELEMENTTYPE val1, ELEMENTTYPE val2), int (*printFunc)(ELEMENTTYPE val1, ELEMENTTYPE val2), friendNode *node, HashTable ** onlineTable) /*先这些后面再加*/

// int chatRoomInit(chatRoomMessage **Message, chatContent **friendMessage, json_object **obj, Friend **Info, Friend **client, Friend ** online, MYSQL **conn, int (*compareFunc)(ELEMENTTYPE val1, ELEMENTTYPE val2), int (*printFunc)(ELEMENTTYPE val), friendNode *node, HashTable ** onlineTable) /*先这些后面再加*/
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
    // 创建一个json对象
    *obj = (json_object*)malloc(sizeof(json_object*)); 
    *obj = json_object_new_object();
    if (obj == NULL) 
    {
        fprintf(stderr, "Failed to create JSON object\n");
        return -1;
    }
    // 将用户列表初始化
    balanceBinarySearchTreeInit(Info, compareFunc, compareFunc1, printFunc);

    balanceBinarySearchTreeInit(client, compareFunc, compareFunc1, printFunc);

    balanceBinarySearchTreeInit(online, compareFunc, compareFunc1, printFunc);
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
    if (*onlineTable)
    {
        printf("你好\n");
        return ret;
    }
    
    hashTableInit(onlineTable, onlineFriNum, compareFunc);
    
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
    snprintf(buffer, sizeof(buffer), "SELECT accountNumber FROM chatRoom WHERE accountNumber = '%s'", accountNumber);
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

    ret = accountRegistration(Message->accountNumber, conn);  /*判断账号是否合法*/
    if (ret == -1)      
    {
        return -1;
    }



    ret = registrationPassword(Message->password);
    if (ret == -1)
    {
        return -1; 
    }

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
    // if (mysql_query(conn, buffer))
    // {
    //     printf("没有该用户\n");
    //     exit(-1);
    // }
    int ret = mysql_query(conn, buffer);
    
    /*判断该账号是否在数据库中*/
    if (ret == 0)       /*是否有该账号*/
    {
        MYSQL_RES* result = mysql_store_result(conn);
        int num_rows = mysql_num_rows(result);
        if (num_rows == 0) 
        {
            printf("没有该账号\n");
            return -1;
        } 
        mysql_free_result(result);
    }
    else 
    {
        printf("查询失败: %d - %s\n", mysql_errno(conn), mysql_error(conn));
        return -1;
    }

    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "SELECT accountNumber FROM chatRoom WHERE accountNumber = '%s' and password = '%s'", Message->accountNumber, Message->password);
    ret = mysql_query(conn, buffer);
    if (ret == 0)       /*是否有该账号*/
    {
        MYSQL_RES* result = mysql_store_result(conn);
        int num_rows = mysql_num_rows(result);
        if (num_rows == 0) 
        {
            printf("账号密码不匹配\n");
            return -1;
        } 
        else 
        {
            printf("账号密码正确登陆成功\n");
            return 0;
        }
        mysql_free_result(result);

    }
    else 
    {
        printf("查询失败: %d - %s\n", mysql_errno(conn), mysql_error(conn));
        return -1;
    }

    return 0;
    
    
}

/* 登录 */  /* 正确返回0， 错误返回-1 */
int chatRoomLogIn(int fd, chatRoomMessage *Message, Friend *client, MYSQL * conn, HashTable * onlineTable, chatHash * onlineHash) 
{
    int ret = 0;

    ret = determineIfItExists(Message, conn);
    if (ret == -1) 
    {
        return -1;
    }

    chatRoomMessage * friendMessage = (chatRoomMessage *)malloc(sizeof(chatRoomMessage));
    memset(friendMessage, 0, sizeof(friendMessage));
    friendMessage->accountNumber = (char *)malloc(ACCOUNTNUMBER);
    memset(friendMessage->accountNumber, 0, ACCOUNTNUMBER);
    friendMessage->name = (char *)malloc(NAMESIZE);
    memset(friendMessage->name, 0, NAMESIZE);

    printf("---name:%s\n", Message->accountNumber);

    /* 先判断是否有这个表没有则创建有则跳过 */
    char buffer[BUFFER_SIZE << 2];
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer),
            "CREATE TABLE IF NOT EXISTS `Friend%s` ("
            "accountNumber VARCHAR(50) PRIMARY KEY,"
            "name VARCHAR(50) NOT NULL UNIQUE,"
            "FOREIGN KEY (accountNumber) REFERENCES chatRoom(accountNumber)"
            " ON DELETE CASCADE"
            " ON UPDATE CASCADE)", Message->accountNumber);

    if (mysql_query(conn, buffer)) 
    {
        printf("系统错误，创建失败: %s\n", mysql_error(conn));
        exit(-1);
    }

    if (chatRoomOnlineTable(onlineHash, onlineTable) > 0)
    {
        printf("插入成功\n");
        
    }
    else
    {
        printf("插入失败请重新插入\n");
        exit(-1);
    }

    

    /* 查询登录人的所有好友，用以之后将其好友放入一个他专属的树中 */
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "SELECT * FROM `Friend%s`", Message->accountNumber);

    ret = mysql_query(conn, buffer);
    
    if (ret == 0) 
    {
        MYSQL_RES* result = mysql_store_result(conn);
        int num_rows = mysql_num_rows(result);
        printf("588 --num_rows:%d\n", num_rows);
        char accountNumber[num_rows][ACCOUNTNUMBER];
        for (int idx = 0; idx < num_rows; idx++)
        {
            memset(accountNumber[idx], 0, ACCOUNTNUMBER);
        }
        
        
        char name[num_rows][NAMESIZE];
        for (int idx = 0; idx < num_rows; idx++)
        {
            memset(name, 0, NAMESIZE);
        }
        
        
        if (num_rows == 0) 
        {
            printf("你没有朋友，是个孤独的人！！\n");
        } 
        else 
        {
            // 获取查询结果集
            MYSQL_ROW row;
            /*获取结果的个数*/
            // 遍历结果集
            while ((row = mysql_fetch_row(result)) != NULL) 
            {
                chatRoomMessage * node = (chatRoomMessage *)malloc(sizeof(chatRoomMessage));
                memset(node, 0, sizeof(chatRoomMessage));
                node->accountNumber = (char *)malloc(ACCOUNTNUMBER);
                memset(node->accountNumber, 0, ACCOUNTNUMBER);
                node->name = (char *)malloc(NAMESIZE);
                memset(node->name, 0, NAMESIZE);
                // 以字符串形式打印每个字段的值
                
                    // if ((row = mysql_fetch_row(result)) != NULL) 
                    // {
                        printf("575 ---- row[0]:%s\n", row[0]);
                        printf("row[1] :%s ", row[1]);
                       
                            // snprintf(accountNumber[idx], ACCOUNTNUMBER, "%s", row[0]);
                            // snprintf(name[idx], NAMESIZE, "%s", row[1]);
                            
                            strncpy(node->accountNumber, row[0], ACCOUNTNUMBER);
                            strncpy(node->name, row[1], NAMESIZE);
                            printf("627 ---- %s\n", node->accountNumber);
                            printf("628 ---- %s\n", node->name);

                            balanceBinarySearchTreeInsert(client, node);
                            printf("631 size- %d\n", client->size);
                            
                            // memset(accountNumber, 0, sizeof(ACCOUNTNUMBER));
                            // memset(name, 0, sizeof(NAMESIZE));
                        // }
                        
                        
                        
                        
                        
                        
                    // }
                    
                    
            }
        }
        // 释放查询结果集
        mysql_free_result(result);
    }
    else 
    {
        printf("查询失败: %d - %s\n", mysql_errno(conn), mysql_error(conn));
        return -1;
    }

    return 0;
}

/*添加好友*/
int chatRoomAppend(chatRoomMessage *Message, json_object *obj, MYSQL * conn, Friend *client) /*查找到提示是否要添加该好友，当点了是时，被添加的客户端接收到是否接受该好友，点否则添加不上，发给他一个添加失败，点接受，则将好友插入到你的数据库表中，同时放入以自己的树中*/
{
    
    
    return 0;

}


/*字符串转整型*/
int getAsciiSum(const char *name) 
{
    int sum = 0;

    while (*name) 
    {
        sum += *name; // 获取当前字符的 ASCII 值并累加
        name++;
    }
    
    return sum;
}


/* 在线列表的插入 */
int chatRoomOnlineTable(chatHash *onlineHash, HashTable *onlineTable)
{
    int ret = getAsciiSum(onlineHash->hashName);
    if (!ret)
    {
        printf("转换失败\n");
        return -1;
    }
    
    if (!hashTableInsert(onlineTable, ret, onlineHash->sockfd))
    {
        printf("哈希 ----ret:%d\n", ret);

        return ret;/*成功*/
    }
    return -1;/*失败*/
}




/*输入好友名字 判断好友是否在线*/
int searchFriendIfOnline(HashTable * onlineTable, char * name)
{
    int hash_name = getAsciiSum(name);
    printf("756--hash_name:%d\n", hash_name);
    if (onlineTable == NULL)
    {
        printf("无在线好友\n");
        return 0;
    }
    if (name == NULL)
    {
        printf("好友名输入错误\n");
        return 0;
    }
    int mapVal = 0;
    
#if 0
    HashTable *pHashtable = (HashTable *) malloc(sizeof(HashTable));
    if (pHashtable == NULL) 
    {
        printf("内存分配失败\n");
        return -1;
    }
    memset((HashTable *)pHashtable, 0, sizeof(HashTable));

    pHashtable->slotNums = SLOTNUMS_MAX;
    pHashtable->compareFunc = compareFunc;
#endif
    printf("781---%p\n", &mapVal);
    if (!hashTableGetAppointKeyValue(onlineTable, hash_name, &mapVal))   /*找到在线好友的句柄*/ 
    {
        printf("784---%d\n", mapVal);
        return mapVal;
    } 
    printf("787---%d\n", mapVal);
    return -1;
}

/* 指定好友是否在线 */
int FriendOnlineOrNot(Friend *client, HashTable *onlineTable, chatHash * onlineHash)
{
    int ret = 1;
    if(balanceBinarySearchTreeIsContainAppointVal(client, onlineHash->hashName))
    {
        if(chatRoomOnlineTable(onlineHash, onlineTable) == 0)
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

/*建立群名 成功返回1 失败返回0*/
int createGroupName(char *groupChatName, MYSQL *conn, chatRoomMessage * Message)
{
    if (groupChatName == NULL)
    {
        return 0;
    }
    int ret = 1;
    char buffer[BUFFER_SIZE_M];
    memset(buffer, 0, BUFFER_SIZE_M);
    /* 先判断是否有这个表没有则创建有则跳过 */
    snprintf(buffer, sizeof(buffer),            
      "CREATE TABLE IF NOT EXISTS `groupChat%s` ("
        "groupChatName VARCHAR(50) PRIMARY KEY )", Message->accountNumber);
    
    if (mysql_query(conn, buffer)) 
    {
        printf("系统错误，创群失败: %s\n", mysql_error(conn));
        exit(-1);
    }
    memset(buffer, 0, BUFFER_SIZE_M);

    /*查询是否有重复群名*/
    snprintf(buffer, sizeof(buffer),
    "SELECT groupChatName FROM `groupChat%s` WHERE groupChatName = '%s'",
    Message->accountNumber, groupChatName);

    if (mysql_query(conn, buffer)) 
    {
        printf("系统错误，查询数据失败: %s\n", mysql_error(conn));
        exit(-1);
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (result == NULL) 
    {
        printf("系统错误，获取查询结果失败: %s\n", mysql_error(conn));
        exit(-1);
    }

    int num_rows = mysql_num_rows(result);
    mysql_free_result(result);

    if (num_rows > 0) 
    {
        printf("群名已存在，请输入其他名称。\n");
        ret = 0; // 返回 0 表示创建群名失败
    }
    else
    {
        snprintf(buffer, sizeof(buffer),
        "INSERT INTO `groupChat%s`(groupChatName) VALUES ('%s')",
        Message->accountNumber, groupChatName);

        if (mysql_query(conn, buffer)) 
        {
            printf("系统错误，插入数据失败: %s\n", mysql_error(conn));
            exit(-1);
        }
        return ret;
    }
                   
    return ret;
    //this

}


/*遍历群名*/
int travelGroupChatName(MYSQL *conn, chatRoomMessage * Message, char * str_travel)
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    char buffer[BUFFER_SIZE_M];
    memset(buffer, 0, sizeof(buffer));
    
    snprintf(buffer, sizeof(buffer),  "CREATE TABLE IF NOT EXISTS `groupChat%s` ("
        "groupChatName VARCHAR(50) PRIMARY KEY)" ,
        Message->accountNumber);
    if (mysql_query(conn, buffer)) 
    {
        printf("系统错误，创表失败: %s\n", mysql_error(conn));
        exit(-1);
    }
    
    memset(buffer, 0, sizeof(buffer));


    char queryBuffer[BUFFER_SIZE_M];
    memset(queryBuffer, 0, sizeof(queryBuffer));
    sprintf(queryBuffer, "SELECT * FROM groupChat%s", Message->accountNumber);

    if (mysql_query(conn, queryBuffer)) 
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }
    res = mysql_use_result(conn);

    // 将查询结果转换为字符串
    buffer[0] = '\0';
    while ((row = mysql_fetch_row(res)) != NULL) 
    {
        //strcat(buffer, "[groupName]\t");
        strcat(buffer, row[0]);
        strcat(buffer, "\t");
    }
    if (strlen(buffer) == 0) // 如果buffer为空，则返回NULL
    {
        return 0;
    }
    // 将结果拷贝到 str_travel 中
    strcpy(str_travel, buffer);
    mysql_free_result(res);
    return 1;
}

/*拉取群成员*/

int pullGroupMembers(MYSQL *conn, char *memberName, chatRoomMessage *Message, char * groupChatName)
{
    int ret = 0;
    MYSQL_RES *res;
    MYSQL_ROW row;
    char insertBuffer[BUFFER_SIZE];
    memset(insertBuffer, 0, sizeof(insertBuffer));
    char newTableName[BUFFER_SIZE_M];
    memset(newTableName, 0, sizeof(newTableName));

    char nbuffer[BUFFER_SIZE_M];
    memset(nbuffer, 0, sizeof(nbuffer));

    // 创建群聊表
    sprintf(insertBuffer, "CREATE TABLE IF NOT EXISTS `%s` ("
        "memberName VARCHAR(50) PRIMARY KEY)", groupChatName);
    
    if (mysql_query(conn, insertBuffer)) 
    {
        fprintf(stderr, "系统错误，创表失败: %s\n", mysql_error(conn));
        return -1;
    }

    // 插入成员到群聊表
    memset(insertBuffer, 0, sizeof(insertBuffer));
    sprintf(insertBuffer, "INSERT INTO `%s` (memberName) VALUES ('%s')", groupChatName, memberName);
    if (mysql_query(conn, insertBuffer)) 
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return -1;
    }

    // 获取成员账号并创建群聊表
    memset(insertBuffer, 0, sizeof(insertBuffer));
    sprintf(insertBuffer, "SELECT accountNumber FROM Friend%s WHERE name = '%s'",Message->accountNumber, memberName);
    if (mysql_query(conn, insertBuffer)) 
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return -1;
    }


    res = mysql_use_result(conn);
    
    while ((row = mysql_fetch_row(res)) != NULL)
    {
        if (row != NULL) 
        {
            char *accountNumber = row[0];
            sprintf(newTableName, "groupChat%s", accountNumber);
           

        } 
        else 
        {
            
            printf("No account found for %s\n", memberName);
            return -1;
        }
    }
    mysql_free_result(res);

    memset(nbuffer, 0, sizeof(nbuffer));
    strncpy(nbuffer, newTableName, sizeof(nbuffer));

    char createTableQery[BUFFER_SIZE];
    memset(createTableQery, 0, sizeof(createTableQery));
    snprintf(createTableQery, sizeof(createTableQery), "CREATE TABLE IF NOT EXISTS `%s` ("
        "groupChatName VARCHAR(50) PRIMARY KEY)", nbuffer);

    if (mysql_query(conn, createTableQery)) 
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return -1;
    }

    printf("Table %s created successfully.\n", nbuffer);


    // 检查群名是否已存在
    
    memset(nbuffer, 0, sizeof(nbuffer));
    strncpy(nbuffer, newTableName, sizeof(nbuffer));
    memset(insertBuffer, 0, sizeof(insertBuffer));

    snprintf(insertBuffer, sizeof(insertBuffer), "SELECT * FROM %s WHERE groupChatName = '%s'", nbuffer, groupChatName);
   
    
    ret = mysql_query(conn, insertBuffer);
    if (ret < 0) 
    {
        fprintf(stderr, "系统错误，查询群名失败: %s\n", mysql_error(conn));
        return -1;
    }
    else if (ret == 0)
    {
        mysql_free_result(mysql_store_result(conn));
        /*该成员没有该群名 ，加入该群*/
        memset(insertBuffer, 0, sizeof(insertBuffer));
        memset(nbuffer, 0, sizeof(nbuffer));
        strncpy(nbuffer, newTableName, sizeof(nbuffer));
        sprintf(insertBuffer, "INSERT INTO `%s` (groupChatName) VALUES ('%s')", nbuffer, groupChatName);
  

        if (mysql_query(conn, insertBuffer)) 
        {
            fprintf(stderr, "系统错误，插入数据失败: %s\n", mysql_error(conn));
            return -1;
        }
    }
    else if (ret > 0)
    {
        /*该成员已在该群*/
        mysql_free_result(mysql_store_result(conn));
      
        return -2; 

    }
    // res = mysql_store_result(conn);
    // int num_rows = mysql_num_rows(res);
    // mysql_free_result(res);

    // if (num_rows > 0) 
    // {
    //     printf("群名已存在，请输入其他名称。\n");
    //     ret = 0; // 返回 0 表示创建群名失败
    // } 
    // else 
    // {
    //     memset(insertBuffer, 0, sizeof(insertBuffer));
    //     sprintf(insertBuffer, "INSERT INTO `%s` (groupChatName) VALUES ('%s')", newTableName, groupChatName);
    //     if (mysql_query(conn, insertBuffer)) 
    //     {
    //         fprintf(stderr, "系统错误，插入数据失败: %s\n", mysql_error(conn));
    //         return -1;
    //     }
        
    //     ret = 1;
    // }

    return 0;
}


/*遍历表中的成员名*/
int iterateTableAndReturnString(char * resultString, MYSQL *conn, char * groupChatName)
{
    char buffer[BUFFER_SIZE_M];
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "SELECT COUNT(*) FROM %s", groupChatName);
    if (mysql_query(conn, buffer) == 0) 
    {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result != NULL) 
        {
            MYSQL_ROW row = mysql_fetch_row(result);
            int rowCount = atoi(row[0]);
            printf("rowCount%d\n", rowCount);
            mysql_free_result(result);
            
            
            
            for (int n = 0; n < rowCount; n++) 
            {
                sprintf(buffer, "SELECT memberName FROM %s LIMIT %d, 1", groupChatName, n);
                if (mysql_query(conn, buffer) == 0) 
                {
                    result = mysql_store_result(conn);
                    if (result != NULL) 
                    {
                        MYSQL_ROW row = mysql_fetch_row(result);
                        if (row != NULL && row[0] != NULL) 
                        {
                            strcat(resultString, row[0]); 
                            strcat(resultString, " "); 
                        }
                        mysql_free_result(result);
                    }
                }
            }
            
            return 1;
        }
    }
    return 0;
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

//获取指定联系人的位置
static void * baseAppointValGetchatRoomSelect(Friend *client, ELEMENTTYPE data)
{
    friendNode * travelNode = client->root;
    int cmp = 0;
    while (travelNode != NULL)
    {
        

        cmp = client->compareFunc1(data, travelNode->data);
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
    obj = json_object_new_object();
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

/*将客户端输入的信息传入json*/ 
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


/*将客户端的信息传入json*/ 
int chatRoomClientLogIn(char * buffer, chatRoomMessage * Message, json_object * obj) 
{
    printf("请输入账号\n");
    scanf("%s", Message->accountNumber);
    printf("请输入密码\n");
    scanf("%s", Message->password);

    chatRoomObjConvert(buffer, Message, obj);

    return 0;
}


/*将chatContent转换成json格式的字符串进行传送*/
int chatRoomObjConvertContent(char * buffer, chatContent * chat, json_object * obj) 
{

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
    obj = json_tokener_parse(buffer);
    if (obj == NULL) 
    {
        fprintf(stderr, "json_tokener_parse failed\n");
        return -1;
    }

    // 从 json 对象中读取字段
    struct json_object * myNameObj = json_object_object_get(obj, "accountNumber");
    if (myNameObj != NULL) 
    {
        const char * myName = json_object_get_string(myNameObj);
        strncpy(chat->myName, myName, sizeof(chat->myName) - 1);
        chat->myName[sizeof(chat->myName) - 1] = '\0';
    }

    struct json_object * friendNameObj = json_object_object_get(obj, "password");
    if (friendNameObj != NULL) 
    {
        const char * friendName = json_object_get_string(friendNameObj);
        strncpy(chat->friendName, friendName, sizeof(chat->friendName) - 1);
        chat->friendName[sizeof(chat->friendName) - 1] = '\0';
    }

    struct json_object * contentObj = json_object_object_get(obj, "name");
    if (contentObj != NULL) 
    {
        const char * content = json_object_get_string(contentObj);
        strncpy(chat->content, content, sizeof(chat->content) - 1);
        chat->content[sizeof(chat->content) - 1] = '\0';
    }

    struct json_object * timeObj = json_object_object_get(obj, "mail");
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

/*解析json字符串的好友姓名*/
const char * resolveFriendName(char * buffer, chatContent * chat)
{
    // 将 json 格式的字符串转换为 json 对象
    struct json_object * obj = json_tokener_parse(buffer);
    if (obj == NULL) 
    {
        fprintf(stderr, "json_tokener_parse failed\n");
        return NULL;
    }
    struct json_object * friendNameObj = json_object_object_get(obj, "friendName");
    if (friendNameObj != NULL) 
    {
        const char * friendName = json_object_get_string(friendNameObj);
        strncpy(chat->friendName, friendName, sizeof(chat->friendName) - 1);
        chat->friendName[sizeof(chat->friendName) - 1] = '\0';
        return chat->friendName;
    }
    return NULL;
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


/*将Message转换成json格式的字符串进行传送*/
int chatHashObjConvert(char * buffer, chatHash * onlineHash, json_object * obj) 
{
    obj = json_object_new_object();
    //  创建 json 对象并添加字段
    if (json_object_object_add(obj, "hashName", json_object_new_string(onlineHash->hashName)) != 0) 
    {
        fprintf(stderr, "json_object_object_add failed for accountNumber\n");
        return -1;
    }

    if (json_object_object_add(obj, "sockfd", json_object_new_int64(onlineHash->sockfd)) != 0) 
    {
        fprintf(stderr, "json_object_object_add failed for password\n");
        return -1;
    }


    // 将 json 对象转换为字符串，并拷贝到 buffer 中
    strcat(buffer, json_object_to_json_string(obj));
    strcat(buffer, "\n");
    buffer[BUFFER_SIZE - 1] = '\0';

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
   
    strcat(buffer, json_object_to_json_string(obj));
    strcat(buffer, "\n");

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

/* 查看人员信息 */
int chatRoomSelect(Friend *client,  ELEMENTTYPE data)
{
    int ret = 0;

    if (client == NULL)
    {
        printf("没有联系人\n");
    }
    
    chatRoomMessage * info = NULL; 
    chatRoomMessage * val = (chatRoomMessage *)data;
    
    info = (chatRoomMessage *)baseAppointValGetchatRoomSelect(client, val);      
    if (info == NULL)
    {
        printf("查无此人\n");
        return -1;
    }
    
    return 0;
}


int logOut( HashTable *pHashtable  , chatRoomMessage * Message, Friend * client, int acceptfd)
{
    int ret = 0;
    int delete_name = getAsciiSum(Message->name);
    hashTableDelAppointKey(pHashtable, delete_name);/*删除在线列表中该用户的信息*/
    balanceBinarySearchTreeDestroy(client);
    printf("客户端%d退出\n", acceptfd);
    
    return ret;
}