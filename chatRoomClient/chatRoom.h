#ifndef __CHAT_ROOM_H_
#define __CHAT_ROOM_H_

#include "balanceBinarySearchTree.h"
#include <json-c/json.h>
#include <mysql/mysql.h>
#include <string.h>
#include <json-c/json_object.h>
#include <time.h>

typedef BalanceBinarySearchTree Friend;     //好友列表
typedef AVLTreeNode friendNode;             //每个好友

typedef struct chatRoomMessage
{
    
    char * accountNumber;      //账号     //登录只需要账号和密码
    char * password;        //密码
    char * name;            //昵称
    char * mail;            //邮箱
   
} chatRoomMessage;

//用于聊天的结构体
typedef struct chatContent
{
    char * friendName;  //好友姓名
    char * myName;      //本人姓名
    char * content;     //聊天内容
    time_t  chatTime;        //聊天时间
}  chatContent;

typedef struct groupChat
{
    char * groupChatName;//群名
    char * membersName;//群成员名字
    time_t  groupChatTime; //群聊天时间
    char * groupChatContent;//群聊记录
}groupChat;

// enum SELECT
// {
//     BUILT = 1,
//     SEEK,
//     DELETE,
//     MODIFY,
//     VIEW_ALL,
//     SIX,
//     QUIT
// };

/* 状态码 */
enum STATUS_CODE
{
    
    ON_SUCCESS,
    NULL_PTR,
    MALLOC_ERROR,
    INVALID_ACCESS,
};

typedef struct chatHash
{
    char * hashName;
    int sockfd;
}chatHash;

typedef struct chatOTO
{
    int otoBuffer;
    int sockfd;
}chatOTO;

typedef struct chatgc
{
    int gcBuffer;
    int sockfd;
}chatgc;

int existenceOrNot(void *arg1, void *arg2);


/*初始化聊天室*/
int chatRoomInit(chatRoomMessage ** Message, groupChat ** groupChatInfo, chatContent **friendMessage, json_object ** obj, Friend * Info, Friend *client, Friend * online, MYSQL ** conn, int (*compareFunc)(ELEMENTTYPE val1, ELEMENTTYPE val2), int (*printFunc)(ELEMENTTYPE val), friendNode * node);    /*先这些后面再加*/

/*注册账号*/
int chatRoomInsert(char *buffer, chatRoomMessage * Message, json_object * obj, MYSQL * conn); /*账号不能跟数据库中的有重复，昵称也是不可重复，通过账号算出一个key（用一个静态函数来计算），这个key便是ID是唯一的，密码要包含大写及特殊字符，最少八位，不然密码不符合条件，将注册好的信息放到数据库中*/

/*登录*/
int chatRoomLogIn(chatRoomMessage * Message, json_object * obj, Friend *client, MYSQL * conn);   /*要将账号，密码的信息传到服务端进行验证是否存在，和密码正确与否，因此要用到json_object*/

/*添加好友*/
int chatRoomAppend(chatRoomMessage *Message, json_object *obj, MYSQL * conn, Friend *Info, Friend *client);   /*查找到提示是否要添加该好友，当点了是时，被添加的客户端接收到是否接受该好友，点否则添加不上，发给他一个添加失败，点接受，则将好友插入到你的数据库表中，同时放入以自己的树中*/

/*输入名字判断好友是否存在*/
int friendIsExit(Friend *Info, ELEMENTTYPE data, char * name);


/*看是否有人在线*/
int chatRoomOnlineOrNot(chatRoomMessage * Message, json_object * obj);    /*每过一段时间向各个客户发一个消息，如果能发出去，判其为在线状态，返回0，不在线则返回0*/

/*建立私聊的联系*/
int chatRoomPrivateChat( char * friendName, int sockfd, chatContent * chat, chatRoomMessage * message); /*建立一个联系只有双方能够聊天*/  /*判断其书否在线， 是否存在这个好友*/

/*建立一个群聊的联系，建立完后将其存储起来*/
int chatRoomGroupChat(chatRoomMessage * Message, json_object * obj);     /*通过UDP进行群发，一些人能够接到*/   /*有点问题后面再想*/

/*删除好友的销毁信息*/
int chatRoomDestroy(chatRoomMessage * Message, json_object * obj, Friend * Info, MYSQL * conn);       /*通过传进来的信息，把数据库中你的好友表中的指定人员信息删除，同时删掉内存中的该信息，释放该内存*/

/*注销账号*/
int chatRoomMessageLogOff(chatRoomMessage * Message, json_object * obj);       /*通过你的账号信息，删除数据库中用户表中你的信息， 因为该表为主表要先删除附表中他的信息，删除完毕后释放通信句柄，退出到主页面*/

/*文件传输*/  /*后面再加*/ /*通过账号信息找到要发送的人，再通过操作将文件发送过去， 接收到提示要不要接受该文件*/
int chatRoomFileTransfer(chatRoomMessage * Message, json_object * obj); 

/*将Message转换成json格式的字符串进行传送*/
int chatRoomObjConvert(char * bufeer, chatRoomMessage * Message, json_object * obj);   

 /*将json格式的字符串转换成原来Message*/
int chatRoomObjAnalyze(char * buffer, chatRoomMessage * Message, json_object * obj); 

/*将客户端的信息传入json*/
int chatRoomClientMeassage(char * buffer, chatRoomMessage * Message, json_object * obj);   

/*用来存放在线人员的昵称以及通信句柄是树结构 通信时会用到*/
int chatRoomOnlineInformation(int sockfd, char * buffer, chatRoomMessage * Message, Friend * online, json_object * obj);   

/*登陆的账号密码写入json*/
int chatRoomClientLogIn(char * buffer, chatRoomMessage * Message, json_object * obj);

/*将chatContent转换成json格式的字符串进行传送*/
int chatRoomObjConvertContent(char * buffer, chatContent * chat, json_object * obj);

int chatRoomObjAnalyzeContent(char * buffer, chatContent * chat, json_object * obj);

/* 退出登录时的资源回收 */
void logoutCleanup(Friend * client, Friend* online, Friend * Info, chatRoomMessage *Message, chatContent *friendMessage, json_object *obj, MYSQL *conn, friendNode *node, groupChat * groupChatInfo) ;

/*将json字符串转化成chatHash结构体*/
int chatHashObjAnalyze(char * buffer, chatHash * onlineHash, json_object * obj);

/*将chatHash结构体转化成json字符串*/
int chatHashObjConvert(char * buffer, chatHash * onlineHash, json_object * obj);


/*将好友信息转换成json类型的字符串*/
int printStructObj(char * buffer, chatRoomMessage * Message, json_object * obj);

/*将json格式的字符串转换成原来Message*/
int objPrintStruct(char * buffer, chatRoomMessage * Message, json_object * obj);

/*将groupChat转换成json格式的字符串进行传送*/
int chatRoomObjGroupChat(char * buffer, groupChat *chatGroup, json_object * obj);

/*将json格式的字符串转换成原来groupChat*/
int chatRoomObjAnalyzeGroupChat(char * buffer, groupChat *chatGroup, json_object * obj);


#endif