#include<stdio.h>
#include <stdlib.h>
#include"doubleLinkList.h"
#include<string.h>
enum STATUS_CODE
{
    NOT_FIND = -1,
    ON_SUCCESS,
    NULL_PTR,
    MALLOC_ERROR,
    INVALID_ACCESS,
};
/* 静态函数只在源文件生效 */
/* 静态函数前置声明*/
static int doubleLinkListAccordAppointValGetPos(doubleLinkList *pList, ELEMENTTYPE val, int *pPos, int(*compareFunc)(ELEMENTTYPE , ELEMENTTYPE));
/* 新建新结点 封装成函数 */
static DoubleLinkNode * createDoubleLinkNode(ELEMENTTYPE val);

/* 链表初始化 */
int doubleLinkListInit(doubleLinkList **pList)
{
    int ret;
    doubleLinkList *list = (doubleLinkList*)malloc(sizeof(doubleLinkList) * 1);
    if(list == NULL)
    {
        return MALLOC_ERROR;
    }
    /* 清空脏数据 */
    memset(list, 0, sizeof(doubleLinkList) * 1);
    list->head = (DoubleLinkNode *)malloc(sizeof(DoubleLinkNode) * 1);
    if(list->head == NULL)
    {
        return MALLOC_ERROR;
    }
    /* 清空脏数据 */
    memset(list->head, 0, sizeof(DoubleLinkNode) * 1);
    list->head->data = 0;
    list->head->next = NULL; 
    /* 虚拟头结点的prev指针置为NULL */
    list->head->prev = NULL;                                 ;
    /* 初始化的时候尾指针 = 头指针 */
    list->tail = list->head;
    
    /* 链表的长度为0 */
    list->len = 0;

    /* 二级指针 */
    *pList = list;
    return ret;
}

/* 链表头插 */
int doubleLinkListHeadInsert(doubleLinkList *pList, ELEMENTTYPE val)
{
    return doubleLinkListAppointPosInsert(pList, 0, val);
}

/* 链表尾插 */
int doubleLinkListTailInsert(doubleLinkList *pList, ELEMENTTYPE val)
{
    return doubleLinkListAppointPosInsert(pList, pList->len, val);
}

static DoubleLinkNode * createDoubleLinkNode(ELEMENTTYPE val)
{
    /* 封装结点 */
    DoubleLinkNode * newNode = (DoubleLinkNode *)malloc(sizeof(DoubleLinkNode) * 1);
    if(newNode == NULL)
    {
        return NULL;
    }
    /* 清楚脏数据 */
    memset(newNode, 0, sizeof(DoubleLinkNode) * 1);

    newNode->data = 0;
    newNode->next = NULL;
    newNode->prev = NULL;
    /* 赋值 */
    newNode->data = val;
    return newNode;

}
/* 链表指定位置插入 */
int doubleLinkListAppointPosInsert(doubleLinkList *pList, int pos, ELEMENTTYPE val)
{
    int ret = 0;
    if(pList == NULL)
    {
        return NULL_PTR;
    }
    if(pos < 0 || pos > pList->len)
    {
        return INVALID_ACCESS;
    }
    /* 新建新结点 封装成函数 */
#if 1
    DoubleLinkNode * newNode =  createDoubleLinkNode(val);
    if(newNode == NULL)
    {
        return NULL_PTR;
    }
#else
    /* 封装结点 */
    DoubleLinkNode * newNode = (DoubleLinkNode *)malloc(sizeof(DoubleLinkNode) * 1);
    if(newNode == NULL)
    {
        return MALLOC_ERROR;
    }
    /* 清楚脏数据 */
    memset(newNode, 0, sizeof(DoubleLinkNode) * 1);

    /* 赋值 */
    newNode->data = val;
#endif
#if 1
    DoubleLinkNode * travelNode = pList->head;
#else
    DoubleLinkNode * travelNode = pList->head->next;
#endif
    int flag = 0;
    //这种情况下需要更改指针
    if(pos == pList->len)
    {
        travelNode = pList->tail;
        flag = 1;
    }
    else{
        while(pos)
        {
            travelNode = travelNode->next;
            pos--;
        }
        travelNode->next->prev = newNode;
    }
    /* 修改结点指向 */
    newNode->next = travelNode->next;
    newNode->prev = travelNode;
    travelNode->next = newNode;

    if(flag)
    {
        pList->tail = newNode;
    }
    /* 修改链表长度*/
    (pList->len)++;
    return ret;
}

/* 头删除 */
int doubleLinkListHeadDel(doubleLinkList *pList)
{
    doubleLinkListDelAppointPos(pList, 1);
}

/* 尾删除 */
int doubleLinkListTailDel(doubleLinkList *pList)
{
    doubleLinkListDelAppointPos(pList, pList->len);
}

/* 链表指定位置删除 */
int doubleLinkListDelAppointPos(doubleLinkList *pList, int pos)
{
    int ret = 0;
    if (pList == NULL)
    {
        return NULL_PTR;
    }
    if(pos <= 0 || pos > pList->len)
    {
        return INVALID_ACCESS;
    }
#if 1
    DoubleLinkNode * travelNode = pList->head;
#else
#endif
    DoubleLinkNode * needDelNode = NULL;
    /* 需要修改尾指针*/
    if(pos == pList->len)
    {
        /* 备份尾指针 */
        DoubleLinkNode * tmpNode = pList->tail;
        /* 移动尾指针 */
        pList->tail = pList->tail->prev;
        needDelNode = tmpNode;
        //pList->tail->next = NULL;
    }
    else
    {
        while(--pos)
        {
            /* 向后移动位置 */
            travelNode = travelNode->next;
            //pos--;
        }    
        //跳出循环找到的是哪一个结点 
        needDelNode = travelNode->next;;
        travelNode->next = needDelNode->next;
        needDelNode->next->prev = travelNode;
    }
#if 0
    if(flag)
    {
        /* 调整尾指针 */
        pList->tail = travelNode;
    }
#endif 
    /* 释放内存 */
    if(needDelNode != NULL)
    {
        free(needDelNode);
        needDelNode = NULL;                                                            
    }
    /* 链表长度减一 */
    (pList->len)--;
    return ret;
}

/* 根据指定的元素得到在链表中的位置 */
static int doubleLinkListAccordAppointValGetPos(doubleLinkList *pList, ELEMENTTYPE val, int *pPos, int(*compareFunc)(ELEMENTTYPE , ELEMENTTYPE))
{
    /* 静态函数只给本源文件的函数使用，不需要判断合法性 */
    int ret = 0;
#if 0
    int pos = 0;
    DoubleLinkNode * trvalNode = pList->head;
#else
    int pos = 1;
    DoubleLinkNode * travelNode = pList->head->next;
#endif
    int cmp = 0;
    while(travelNode != NULL)
    {
#if 0
        if(trvalNode->data == val)
        {
            /* 解引用 */
            *pPos = pos;
            return pos;
        }
#else
        cmp = compareFunc(val, travelNode->data);
        if(cmp == 0)
        {
            /* 解引用 */
            *pPos = pos;
            return pos;
        }
#endif
        travelNode = travelNode->next;
        pos++;
    }
    *pPos = NOT_FIND;
    return NOT_FIND;
}

/* 删除指定数据 */
int doubleLinkListDelAppointData(doubleLinkList *pList, ELEMENTTYPE val, int(*compareFunc)(ELEMENTTYPE, ELEMENTTYPE))
{
    int ret = 0;
    /* 元素在链表中的位置 */
    int pos = 0;
    /* 链表长度 */
    int size = 0;
    // DoubleLinkNode * travelNode = pList->head->next;
    while(doubleLinkListGetLength(pList, &size) && pos != NOT_FIND)
    {
#if 0
        ret = compareFunc(val, travelNode->data);
        travelNode = travelNode->next;
        /* 根据指定的元素得到在链表中的位置 */
        if(compareFunc(val, travelNode->data))
        {
            doubleLinkListAccordAppointValGetPos(pList, val, &pos);
            doubleLinkListDelAppointPos(pList, pos);
            printf("%d\n", ret);
            ret = 0;
            pos--;
        }
#endif
        doubleLinkListAccordAppointValGetPos(pList, val, &pos, compareFunc);
        doubleLinkListDelAppointPos(pList, pos);       
    }
    return ret;
}

/* 获取链表长度 */
int doubleLinkListGetLength(doubleLinkList *pList, int *pSize)
{
    //int ret = 0;
    if(pList == NULL)
    {
        return NULL_PTR;
    }
    if(pSize)
    {
        *pSize = pList->len;
    }
    /* 返回链表的长度 */
    return pList->len;

}
/* 链表的销毁 */
int doubleLinkListDestory(doubleLinkList *pList)
{
    int ret = 0;
    /* 使用头删 */
    int size = 0;
    while(doubleLinkListGetLength(pList, &size))
    {
        doubleLinkListHeadDel(pList);
    }
    if(pList->head != NULL)
    {
        free(pList->head);
        /* 指针位置置为NULLL */
        pList->head = NULL;
        pList->tail = NULL;
    }
    return ret;
}

/* 链表遍历接口 */
int doubleLinkListForeach(doubleLinkList *pList, int (*printFunc)(ELEMENTTYPE))
{
    int ret = 0;                                                                                                                                                                                                                                                                                              
    if(pList == NULL)
    {
        return NULL_PTR;
    }
#if 0
    /* travelNode指向虚拟头结点 */
    DoubleLinkNode * travelNode = pList->head;
    while(travelNode->next != NULL)
    {
        travelNode = travelNode->next;
       printf("travelNode->data:%d\n", travelNode->data);
    } 
#else
    /* travelNode指向链表第一个元素 */
    DoubleLinkNode * travelNode = pList->head->next;
    while(travelNode != NULL)
    {
#if 0
       printf("travelNode->data:%d\n", travelNode->data);
#else
       /* 包装器 钩子 回调函数*/
       printFunc(travelNode->data);
#endif
       travelNode = travelNode->next;
    } 
#endif
    return ret;
}

/* 逆序遍历 */
int doubleLinkListReverseForeach(doubleLinkList *pList, int (*printFunc)(ELEMENTTYPE))
{
    int ret = 0;
    if(pList == NULL)
    {
        return NULL_PTR;
    }

    DoubleLinkNode * travelNode = pList->tail;
     
    while(travelNode != pList->head)
    {
        printFunc(travelNode->data);
        travelNode = travelNode->prev;
    }
    return ret;
}
/* 获取头位置的值 */
int doubleLinkListGetHeadVal(doubleLinkList *pList, ELEMENTTYPE *pVal)
{
    #if 0
    
    int ret = 0;
    if (pList == NULL)
    {
        return NULL_PTR;
    }
    if(pVal)
    {
        *pVal = pList->head->next->data;
    }
    return ret;
    #endif
    doubleLinkListGetAppointVal(pList, 1, pVal);
}

/* 获取链表尾位置的值 */
int doubleLinkListGetTailVal(doubleLinkList *pList, ELEMENTTYPE *pVal)
{
    doubleLinkListGetAppointVal(pList, pList->len, pVal);
}

/* 获取链表 指定位置的值 */
int doubleLinkListGetAppointVal(doubleLinkList *pList, int pos, ELEMENTTYPE *pVal)
{
    #if 0
    #else
    int ret = 0;
    if (pList == NULL)
    {
        return NULL_PTR;
    }
    if(pos <= 0 || pos > pList->len)
    {
        return INVALID_ACCESS;
    }
    DoubleLinkNode * travelNode = pList->head;
    while(pos)
    {
        pos--;
        travelNode = travelNode->next;
    }
    *pVal = travelNode->data;
    return ret;
    #endif
}

