#ifndef __THREAD_POOL_H_
#define __THREAD_POOL_H_

#include <pthread.h>

typedef struct task_t
{
    /* 钩子 - 回调函数. */
    void *(*worker_hander)(void *arg);
    /* 参数 */
    void *arg;
} task_t;

/* 线程池结构体 */
typedef struct threadpool_t
{
    /* 任务队列 */
    task_t * taskQueue;
    /* 任务队列的容量 */
    int queueCapacity;
    /* 任务队列的任务数 */
    int queueSize;
    /* 任务队列的队头 */
    int queueFront;
    /* 队伍队列的队尾 */
    int queueRear;

    /* 线程池中的线程 */
    pthread_t *threadIds;
    /* 线程池中的管理线程 */
    pthread_t managerThread;

    /* 最小的线程数 */
    int minThreads;
    /* 最大的线程数 */
    int maxThreads;

    /* 干活的线程数 - 干活的线程数不等于存活的线程数. */
    int busyThreadNums;
    /* 存活的线程数 */
    int liveThreadNums;

    /* 锁 - 维护整个线程池. */
    pthread_mutex_t mutexpool;
    /* 锁 - 只维护干活的线程 */
    pthread_mutex_t mutexBusy;

    /* 条件变量 : 任务队列有任务可以消费 */
    pthread_cond_t notEmpty;
    /* 条件变量 : 任务队列有空位 可以继续放. */
    pthread_cond_t notFull;

    /* 离开的线程数 */
    int exitThreadNums;
    
    /* 关闭线程池 */
    int shutDown;
}threadpool_t;

/* 线程池初始化 */
int threadPoolInit(threadpool_t *pool, int minThreads, int maxThreads, int queueCapacity);

/* 线程池添加任务 */
int threadPoolAddTask(threadpool_t *pool, void *(worker_hander)(void *), void *arg);

/* 线程池销毁 */
int threadPoolDestroy(threadpool_t *pool);


#endif // __THREAD_POOL_H_