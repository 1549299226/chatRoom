#ifndef __CHAT_ROOM_INTERFACE_H_
#define __CHAT_ROOM_INTERFACE_H_


enum FIRST_OPTION
{
    LOGIN = 1,
    ENTER,
};

enum LOGOUT_OPTION
{
    YES = 1,
    NO,
};

enum MAIN_OPTION
{
    APPEND = 1,
    CHECK,
    INITIATE,
    DELETE,
    QUIT ,
    LOGOUT = 0,
};




/* 欢迎界面以及选择界面 */
void welcomeInterface();

/* 注册登录界面 */
void fristInterface();

/* 主界面 */
void mainInterface();

/* 注册界面 */
void loginInterface();

/* 登录界面 */
void enterInterface();

/* 注销界面 */
void logoutInterface();

/* 私聊界面 */
void privateChatInterface();

/* 群聊界面 */
void groupChatInterface();

/* 退出界面 */
void quitChatInterface();


#endif

