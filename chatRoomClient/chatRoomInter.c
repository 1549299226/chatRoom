#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "chatRoomInter.h"
#include <string.h>


/* 欢迎界面以及选择界面 */
void welcomeInterface()
{

    system("clear");
    printf("\033[1;47m╔═════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║            欢迎来到超哥架构师之路的第一个项目           ║\033[0m\n");
    printf("\033[1;47m║             带领着第三组三个挂件实现的聊天室            ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║   在即将到来的新春之际，请允许我组向您致以最诚挚的祝福  ║\033[0m\n");
    printf("\033[1;47m║              愿您在新的一年暴富🧧!                      ║\033[0m\n");
    printf("\033[1;47m║                            被爱💏!                      ║\033[0m\n");
    printf("\033[1;47m║                            好运常在🤩!                  ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                ps:（按回车键继续哦）                    ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m╚═════════════════════════════════════════════════════════╝\033[0m\n");

    char any;
    getchar();
}

/* 注册登录界面 */
void fristInterface()
{
    system("clear");
    printf("\033[1;47m╔═════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                     请选择您的选项                      ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                   1、注  册                             ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                   2、登  录                             ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║          如果您是第一次使用的用户请选择注册选项         ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m╚═════════════════════════════════════════════════════════╝\033[0m\n");

}

/* 主界面 */
void mainInterface()
{
    system("clear");
    printf("\033[1;47m╔═════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                 欢迎使用聊天室(Alpha)                    ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║            开始跟朋友们交流吧! Have a nice day!          ║\033[0m\n");
    printf("\033[1;47m║                                                        ║\033[0m\n");
    printf("\033[1;47m║                   1、 添 加 好 友                       ║\033[0m\n");
    printf("\033[1;47m║                   2、 聊 天 功 能                       ║\033[0m\n");
    printf("\033[1;47m║                   3、 删 除 好 友                       ║\033[0m\n");
    printf("\033[1;47m║                   0、 退 出 登 录                       ║\033[0m\n");
    printf("\033[1;47m║                   X、 注 销 账 号                       ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m╚═════════════════════════════════════════════════════════╝\033[0m\n");
}

/* 注册界面 */
void loginInterface()
{
    
    
    system("clear");
    printf("\033[1;47m╔═════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                 恭喜您！账号注册完毕！                    ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                请熟记您的账号、密码或拍照保存              ║\033[0m\n");
    printf("\033[1;47m║               切勿将账号外接，以防从事非法活动             ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                   输入回车进入下一个页面                  ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m╚═════════════════════════════════════════════════════════╝\033[0m\n");
    
    
    char any;
    getchar();
}

/* 登录界面 */
void enterInterface()
{
    system("clear");
    printf("\033[1;47m╔═════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║           欢迎加入本聊天室，                            ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                 希望您在这里感受到温馨和舒适             ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║             如果有任何问题，请尽快联系超哥                ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                  ps:（按回车键继续哦）                   ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m╚═════════════════════════════════════════════════════════╝\033[0m\n");
    
    char any;
    getchar();

}

/* 注销界面 */
void logoutInterface()
{
    system("clear");
    printf("\033[1;47m╔═════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║            为确保您不是误触，请确认是否要注销           ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                   1、确 认 注 销                        ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                   2、返 回 上 层                        ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m╚═════════════════════════════════════════════════════════╝\033[0m\n");
    
    int choice;
    // while(getchar()!='\n');
    scanf("%d",&choice);
    switch(choice)
    {
        case YES:
            system("clear");
            printf("\033[1;47m╔═════════════════════════════════════════════════════════╗\033[0m\n");
            printf("\033[1;47m║                                                         ║\033[0m\n");
            printf("\033[1;47m║                                                         ║\033[0m\n");
            printf("\033[1;47m║             感谢您曾经选择过此聊天室，                  ║\033[0m\n");
            printf("\033[1;47m║             如果您有对本聊天室的指导意见，              ║\033[0m\n");
            printf("\033[1;47m║             请发送至👉 1549299226@qq.com                ║\033[0m\n");
            printf("\033[1;47m║                                                         ║\033[0m\n");
            printf("\033[1;47m║                                                         ║\033[0m\n");
            printf("\033[1;47m║                                                         ║\033[0m\n");
            printf("\033[1;47m║             最后祝您生活愉快，                          ║\033[0m\n");
            printf("\033[1;47m║                 With the Best wishes!                   ║\033[0m\n");
            printf("\033[1;47m║                                                         ║\033[0m\n");
            printf("\033[1;47m║                                                         ║\033[0m\n");
            printf("\033[1;47m╚═════════════════════════════════════════════════════════╝\033[0m\n");
            exit(1);
        
        case NO:
            mainInterface();

        default:
            logoutInterface();
    }
}

/* 私聊界面 */
void privateChatInterface()
{

}

/* 群聊界面 */
void groupChatInterface()
{

}

/* 退出界面 */
void quitChatInterface()
{
    system("clear");
    printf("\033[1;47m╔═════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                欢乐的时光总是短暂的                     ║\033[0m\n");
    printf("\033[1;47m║                 又到了说再见的时候                      ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                    感谢您的使用，                       ║\033[0m\n");
    printf("\033[1;47m║                愿您身体安康，心想事成                   ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m║                                                         ║\033[0m\n");
    printf("\033[1;47m╚═════════════════════════════════════════════════════════╝\033[0m\n");  
    exit(1);
}



