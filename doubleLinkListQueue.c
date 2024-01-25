#include "doubleLinkListQueue.h"


/* 队列的初始化 */
int doubleLinkListQueueInit(doubleLinkListQueue **pQueue)
{
    return doubleLinkListInit(pQueue);
}

/* 入队 */
int doubleLinkListQueuePush(doubleLinkListQueue *pQueue, ELEMENTTYPE val)
{
    return doubleLinkListTailInsert(pQueue, val);
}

/* 队头元素 */
int doubleLinkListQueueTop(doubleLinkListQueue *pQueue, ELEMENTTYPE * pVal)
{
    return doubleLinkListGetHeadVal(pQueue, pVal);
}

/* 队尾元素 */
int doubleLinkListQueueRear(doubleLinkListQueue *pQueue, ELEMENTTYPE * pVal)
{
    return doubleLinkListGetTailVal(pQueue, pVal);
}

/* 队列出列 */
int doubleLinkListQueuePop(doubleLinkListQueue *pQueue)
{
    return doubleLinkListHeadDel(pQueue);
}

/* 队列大小 */
int doubleLinkListQueueGetSize(doubleLinkListQueue *pQueue, int *pSize)
{
    return doubleLinkListGetLength(pQueue, pSize);
}

/* 队列是否为空 */
int doubleLinkListQueueIsEmpty(doubleLinkListQueue *pQueue)
{
    int size = 0;
    doubleLinkListGetLength(pQueue, &size);
    return size == 0 ? 1 : 0;
}

/* 队列销毁 */
int doubleLinkListQueueDestorty(doubleLinkListQueue *pQueue)
{
    return doubleLinkListDestory(pQueue);
}