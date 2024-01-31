#ifndef _THREADPOOL_
#define _THREADPOOL_

#include <pthread.h>

/*任务结构体*/
typedef struct task_t
{
    void * (* work_handle)(void *arg);  /**/
    void * arg;
} task_t;

/*线程池结构体*/
typedef struct threadPool_t
{
    task_t * taskQueue;     /*任务队列*/
    int queueCapacity;      /*任务队列容量*/
    int queueSize;          /*任务队列当前任务数量*/
    int queueFront;         /*任务队列的头*/
    int queueRear;          /*任务队列的队尾*/

    pthread_t managerId;    /*管理者线程id*/
    pthread_t  * threadIds; /*工作线程id*/

    int minThreadNums;      /*最小的线程数*/
    int maxThreadNums;      /*最大的线程数*/
    int busyThreadNums;     /*正在工作的线程数*/
    int liveThreadNums;     /*存活的线程数*/
    int exitNums;    /*需要销毁的线程数*/

    pthread_mutex_t mutexPool;  /*锁整个的线程池*/
    pthread_mutex_t mutexbusy;  /*锁住busynums线程*/

    pthread_cond_t notFull;     /* 条件变量 - 消费者向生产者发送 目的: 可以继续生产 */
    pthread_cond_t notEmpty;    /* 条件变量 - 生产者向消费者发送 目的: 可以继续消费 */

    int shutdown;               /*是否要销毁线程池 销毁为1 不销毁为0*/


}threadPool_t;

/*初始化线程池*/
int threadPoolInit(threadPool_t * pool, int minThreadNums, int maxThreadNums, int queueCapacity);

/* 添加任务 */
int threadPoolAddTask(threadPool_t * pool, void *(*worker_hander)(void * arg), void *arg);

/*获取线程池中工作的个数*/
int threadPoolBusyNum(threadPool_t * pool);

/*获取线程池中活着的线程数*/
int threadPoolLiveNum(threadPool_t * pool);

/*线程池的销毁*/
int threadPoolDestroy(threadPool_t * pool);

#endif