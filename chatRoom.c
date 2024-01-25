#include <stdio.h>
#include "chatRoom.h"
#include <stdlib.h>
#include <error.h>
#include <string.h>
#include <strings.h>

#define PASSWORD_MAX 8
#define PASSWORD_MIN 6
#define MAILSIZE 20
#define NAMESIZE 12
#define ACCOUNTNUMBER 6

/*初始化聊天室*/
int chatRoomInit(chatRoomMessage * Message, json_object * obj, Friend * Info, int (*compareFunc)(ELEMENTTYPE val1, ELEMENTTYPE val2), int (*printFunc)(ELEMENTTYPE val), friendNode * node)    /*先这些后面再加*/
{
    int ret = 0;

    Message->name = (char *)malloc(sizeof(char) * NAMESIZE);
    if (Message->name == NULL)
    {
        return MALLOC_ERROR;
    }
    /*清楚脏数据*/
    bzero(Message->name, sizeof(char) * NAMESIZE);


    Message->accountNumber = (char*)malloc(sizeof(char) * ACCOUNTNUMBER);
    if (Message->accountNumber == NULL)
    {
        return MALLOC_ERROR;
    } 

    Message->mail = (char *)malloc(sizeof(char) * MAILSIZE);
    if (Message->mail == NULL)
    {
        return MALLOC_ERROR;
    }
    bzero(Message->mail, sizeof(char) * MAILSIZE);

    Message->password = (char*)malloc(sizeof(char) * PASSWORD_MAX);
    if (Message->password == NULL)
    {
        return MALLOC_ERROR;
    }
    bzero(Message->password, PASSWORD_MAX);
    
    obj = json_object_new_object();

    balanceBinarySearchTreeInit(&Info, compareFunc, printFunc);

    node = (friendNode*)malloc(sizeof(friendNode));
    if (node == NULL)
    {
        return MALLOC_ERROR;
    }
    bzero(node, sizeof(friendNode));

    node->data = (chatRoomMessage*)malloc(sizeof(chatRoomMessage));
    if (node->data == NULL)
    {
        return MALLOC_ERROR;
    }
    node->height = 0;
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;

    return ret;
}


/*注册账号*/
int chatRoomInsert(chatRoomMessage * Message, json_object * obj) /*账号不能跟数据库中的有重复，昵称也是不可重复，通过账号算出一个key（用一个静态函数来计算），这个key便是ID是唯一的，密码要包含大写及特殊字符，最少八位，不然密码不符合条件，将注册好的信息放到数据库中*/
{
    int letter = 0;   //记录是否有字母
    int figure = 0;    //记录是否有数字
    int specialCharacter = 0;   //记录是否有特殊字符
    printf("请输入账号：(六位0-9的数字)\n");
    scanf("%s", Message->accountNumber);
    for (int idx = 0; idx < sizeof(Message->accountNumber); idx++)
    {
        if (idx > '9' || idx < '0')
        {
            s
        }
        
    }
    
    printf("请输入密码：(六到八位，包括大小写，特殊字符，及数字)\n");
    scanf("%s", Message->password);
    printf("请输入你的邮箱\n");
    scanf("%s", Message->mail);

}

/*登录*/
int chatRoomLogIn(chatRoomMessage * Message, json_object * obj)   /*要将账号，密码的信息传到服务端进行验证是否存在，和密码正确与否，因此要用到json_object*/
{

}

/*添加好友*/
int chatRoomAppend(chatRoomMessage * Message, json_object * obj, Friend * Info)   /*查找到提示是否要添加该好友，当点了是时，被添加的客户端接收到是否接受该好友，点否则添加不上，发给他一个添加失败，点接受，则将好友插入到你的数据库表中，同时放入以自己的树中*/
{

}

/*看是否有人在线*/
int chatRoomOnlineOrNot(chatRoomMessage * Message, json_object * obj)    /*每过一段时间向各个客户发一个消息，如果能发出去，判其为在线状态，返回0，不在线则返回0*/
{

}

/*建立私聊的联系*/
int chatRoomPrivateChat(chatRoomMessage * Message, json_object * obj)   /*建立一个联系只有双方能够聊天*/  /*判断其书否在线， 是否存在这个好友*/
{

}

/*建立一个群聊的联系，建立完后将其存储起来*/
int chatRoomGroupChat(chatRoomMessage * Message, json_object * obj)     /*通过UDP进行群发，一些人能够接到*/   /*有点问题后面再想*/
{

}

/*删除好友的销毁信息*/
int chatRoomDestroy(chatRoomMessage * Message, json_object * obj)       /*通过传进来的信息，把数据库中你的好友表中的指定人员信息删除，同时删掉内存中的该信息，释放该内存*/
{

}

/*注销账号*/
int chatRoomMessageLogOff(chatRoomMessage * Message, json_object * obj)       /*通过你的账号信息，删除数据库中用户表中你的信息， 因为该表为主表要先删除附表中他的信息，删除完毕后释放通信句柄，退出到主页面*/
{

}

/*文件传输*/  /*后面再加*/
int chatRoomFileTransfer(chatRoomMessage * Message, json_object * obj) /*通过账号信息找到要发送的人，再通过操作将文件发送过去， 接收到提示要不要接受该文件*/
{
    
}