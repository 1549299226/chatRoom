#ifndef _DOUBLELINKLIST_QUEUE_H_
#define _DOUBLELINKLIST_QUEUE_H_

#include "doubleLinkList.h"
typedef doubleLinkList doubleLinkListQueue;
/* 队列的初始化 */
int doubleLinkListQueueInit(doubleLinkListQueue **pQueue);

/* 入队 */
int doubleLinkListQueuePush(doubleLinkListQueue *pQueue, ELEMENTTYPE val);

/* 队头元素 */
int doubleLinkListQueueTop(doubleLinkListQueue *pQueue, ELEMENTTYPE * pVal);

/* 队尾元素 */
int doubleLinkListQueueRear(doubleLinkListQueue *pQueue, ELEMENTTYPE * pVal);

/* 队列出列 */
int doubleLinkListQueuePop(doubleLinkListQueue *pQueue);

/* 队列大小 */
int doubleLinkListQueueGetSize(doubleLinkListQueue *pQueue, int *pSize);

/* 队列是否为空 */
int doubleLinkListQueueIsEmpty(doubleLinkListQueue *pQueue);

/* 队列销毁 */
int doubleLinkListQueueDestorty(doubleLinkListQueue *pQueue);
#endif