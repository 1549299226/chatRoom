#ifndef _COMMON_H_
#define _COMMO_H_

#define ELEMENTTYPE void*

/* 链表结点取别名 */
typedef struct DoubleLinkNode
{ 
    ELEMENTTYPE data;
    /* 指向前一个结点的指针 */
    struct DoubleLinkNode * prev;
    /* 指向下一个结点的指针 */
    struct  DoubleLinkNode * next;
}DoubleLinkNode;


/* 链表 */
typedef struct doubleLinkList
{
    /* 链表的虚拟头结点*/
    DoubleLinkNode * head;
    DoubleLinkNode * tail;    /* 为什么尾指针不需要分配空间 */
    /* 链表长度 */
    int len;
} doubleLinkList;
#endif