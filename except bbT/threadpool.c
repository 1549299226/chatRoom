#include <stdio.h>
#include "threadpool.h"
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

enum STATUS_CODE
{
    ON_SUCCESS,
    NULL_PTR,
    MALLOC_ERROR,
    INVALID_ACCESS,
    THREAD_CREATE_ERR,
    UNKNOWN_ERROR,
};

#define DEFAULT_MIN_THREADS 5
#define DEFAULT_MAX_THREADS 100
#define DEFAULT_MAX_QUEUES 100
#define TIME_INTERVAL   5

/* 扩容/缩容每次增加或减少的默认线程池 */
#define DEFAULT_VARY_THREADS    3


static void * thread_handle(void * arg);
static void  * manager_handle(void * arg);
static void pthreadExitClearThreadIndex(threadPool_t * pool);

/*从线程池中退出清理资源*/
static void pthreadExitClearThreadIndex(threadPool_t * pool)
{
    pthread_t tid = pthread_self();
    /* 需要判断当前线程是在数组threads对应的哪一个索引里面. */
    for (int idx = 0; idx < pool->maxThreadNums; idx++)
    {
        if (pool->threadIds[idx] == pthread_self())
        {
            /* 将需要退出的线程 对应的索引清空. */
            pool->threadIds[idx] = 0;
            printf("threadExit() called, %ld exiting..\n", tid);
            break;
        }
    }
    pthread_exit(NULL);
}

/*本质是一个消费者函数*/
static void * thread_handle(void * arg)
{
    threadPool_t * pool = (threadPool_t *)arg;

    while (1)
    {
        pthread_mutex_lock(&(pool->mutexPool));
        /*没有任务消费的时候*/
        while (pool->queueSize == 0 && pool->shutdown == 0)
        {
            /* 等待一个条件变量. 生产者发送过来的. */
            pthread_cond_wait(&(pool->notEmpty), &(pool->mutexPool));
            if (pool->exitNums > 0)
            {
                /*需要销毁的线程数减一*/
                pool->exitNums--;
                if (pool->liveThreadNums > pool->minThreadNums)
                {
                    pool->liveThreadNums--;
                    /*解锁 避免死锁*/
                    pthread_mutex_unlock(&(pool->mutexPool));
                    /*线程退出*/
                    pthreadExitClearThreadIndex(pool);

                }
            }
        }
        /*判断线程池是否关闭*/
        if (pool->shutdown == 1)
        {
            /*解锁 -- 避免死锁*/
            pthread_mutex_unlock(&(pool->mutexPool));
            /*线程池退出*/
            pthreadExitClearThreadIndex(pool);
        }
        /* 从任务队列中取数据 -- 从队列的对头取数据 */
        task_t task = pool->taskQueue[pool->queueFront];
        /* 任务队列的任务数减一. */
        pool->queueSize--;
        /* front 向后移动. */
        pool->queueFront = (pool->queueFront + 1) % (pool->queueCapacity);

        /* 发一个信号给生产者 告诉他可以继续生产. */
        pthread_cond_signal(&pool->notFull);
        /*解锁*/
        pthread_mutex_unlock(&(pool->mutexPool));

        printf("thread %ld start working...\n", pthread_self());

        /* 为了提升我们性能, 再创建一把只维护busyNum属性的锁. */
        pthread_mutex_lock(&pool->mutexbusy);
        pool->busyThreadNums++;
        pthread_mutex_unlock(&pool->mutexbusy);

        /*执行钩子函数*/
        task.work_handle(task.arg);
        /*释放内存*/
        free(task.arg);
        task.arg = NULL;

        printf("thread %ld end working...\n", pthread_self());
        pthread_mutex_lock(&pool->mutexbusy);
        pool->busyThreadNums--;
        pthread_mutex_unlock(&pool->mutexbusy);       
    }
    pthread_exit(NULL);
}

/*管理者线程*/
static void  * manager_handle(void * arg)
{
    /* 强制类型转换. */
    threadPool_t *pool = (threadPool_t *)arg;

    while (pool->shutdown == 0)
    {
        sleep(TIME_INTERVAL);
        pthread_mutex_lock(&(pool->mutexPool));
        /*任务队列任务数*/
        int taskNums = pool->queueSize;
        /*存活的线程数*/
        int liveThreadNums = pool->liveThreadNums;
        pthread_mutex_unlock(&(pool->mutexPool));

        pthread_mutex_lock(&(pool->mutexbusy));
        /*忙的线程数*/
        int busyThreadNums = pool->busyThreadNums;
        pthread_mutex_unlock(&(pool->mutexbusy));

        /* 扩容: 扩大线程池里面的线程数 (上限不要超过maxThreads) */
        /* 任务队列任务数 > 存活的线程数 && 存活的线程数 < maxThreads */
        if (taskNums > liveThreadNums && liveThreadNums < pool->maxThreadNums)
        {
            pthread_mutex_lock(&(pool->mutexPool));
            /*计数*/
            int count = 0;
            int ret = 0;
            /*一次扩3个线程*/
            for (int idx = 0; idx < pool->maxThreadNums && count < DEFAULT_VARY_THREADS
                &&liveThreadNums <= pool->maxThreadNums; idx++
                )
            {
                /*能够使用的索引位置*/
                if (pool->threadIds[idx] == 0)
                {
                    ret = pthread_create(&(pool->threadIds[idx]), NULL, thread_handle, pool);
                    if (ret != 0)
                    {
                        perror("thread create error");
                        sleep(5);/*to do..*/
                    }
                    /*计数加一*/
                    count++;
                    /*存活的线程数加一*/
                    pool->liveThreadNums++;
                }

            }
            pthread_mutex_unlock(&(pool->mutexPool));
        }
        /* 缩容: 减少线程池里面的线程数 (下限不要低于minThreads) */
        /* 忙的线程数 * 2 < 存活的线程数 && 存活的线程数 > minThreads */
        if ((busyThreadNums >> 1) < liveThreadNums && liveThreadNums > pool->minThreadNums)
        {
            pthread_mutex_lock(&(pool->mutexPool));
            /*离开的线程数*/
            pool->exitNums = DEFAULT_VARY_THREADS;
            /*解锁*/
            pthread_mutex_unlock(&(pool->mutexPool));

            for (int idx = 0; idx < DEFAULT_VARY_THREADS; idx++)
            {
                /*发信号让线程自杀*/
                pthread_cond_signal(&(pool->notEmpty));
            }
        }       
    }
    /* 线程关闭 */
    pthread_exit(NULL);
}
/*初始化线程池*/
int threadPoolInit(threadPool_t * pool, int minThreadNums, int maxThreadNums, int queueCapacity)
{
    if (pool == NULL)
    {
        return NULL_PTR;
    }

    /*do ... while 循环*/
    do
    {
        /*判断线程数合法性*/
        if (minThreadNums <= 0 || maxThreadNums <= 0 || minThreadNums > maxThreadNums)
        {
            minThreadNums = DEFAULT_MIN_THREADS;
            maxThreadNums = DEFAULT_MAX_THREADS;
        }
        /*判断传进来的容量合法性*/
        if (queueCapacity < 0)
        {
            queueCapacity = DEFAULT_MAX_QUEUES;
        }
        pool->minThreadNums = minThreadNums;
        pool->maxThreadNums = maxThreadNums;
        /*队列的容量*/
        pool->queueCapacity = queueCapacity;
        /*队列的任务数大小*/
        pool->queueSize = 0;
        pool->exitNums = 0;
        pool->liveThreadNums = minThreadNums;
        pool->busyThreadNums = 0;

        /*任务队列初始化*/
        pool->taskQueue = (struct task_t*)malloc(sizeof(task_t) * (pool->queueCapacity));
        if (pool->taskQueue == NULL)
        {
            perror("malloc error");
            break;  /*不能直接结束，资源未回收*/
        }
        memset(pool->taskQueue, 0, sizeof(task_t) * pool->queueCapacity);
        /*循环队列队头位置*/
        pool->queueFront = 0;
        /* 循环队列队尾位置 */
        pool->queueRear = 0;
        pool->taskQueue->arg = NULL;

        /*工作线程初始化*/
        pool->threadIds = (pthread_t *)malloc(sizeof(pthread_t) * pool->maxThreadNums);
        if (pool->threadIds == NULL)
        {
            perror("malloc error");
            break;
        }
        bzero(pool->threadIds, sizeof(pthread_t) * pool->maxThreadNums);

        /*工作线程创建*/
        int ret = 0;
        for (int idx = 0; idx < pool->minThreadNums; idx++)
        {
            ret = pthread_create(&(pool->threadIds[idx]), NULL, thread_handle, (void*)pool);
            if (ret != 0)
            {
                perror("pthread_create error");
                break;
            }
        }

        /*管理者线程的创建*/
        ret = pthread_create(&(pool->managerId), NULL, manager_handle, (void*)pool);
        if (ret != 0)
        {
            perror("pthread_create error");
            break;
        }
        
        /*锁和条件变量*/
        if (
            pthread_mutex_init(&(pool->mutexPool), NULL) != 0 ||
            pthread_mutex_init(&(pool->mutexbusy), NULL) != 0
            )
        {
            perror("nutex error");
            break;
        }

        if (
            pthread_cond_init(&(pool->notFull), NULL) != 0    || 
            pthread_cond_init(&(pool->notEmpty), NULL) != 0
        )
        {
            perror("cond error ");
            break;
        }

        /*销毁线程池 标志位置0*/
        pool->shutdown = 0;
        return ON_SUCCESS;
    }while(0);

    /* 程序到达这边意味着上面初始化流程出现了错误 */
    /*释放内存*/
    if (pool->taskQueue != NULL)
    {
        free(pool->taskQueue);
        pool->taskQueue = NULL;
    }

    /* 回收工作线程的资源*/
    if (pool->threadIds)
    {
        /*阻塞回收工作线程资源*/
        for (int idx = 0; idx < pool->minThreadNums; idx++)
        {
            if (pool->threadIds[idx] != 0)
            {
                pthread_join(pool->threadIds[idx], NULL);
            }
        }
        /*释放内存*/
        free(pool->threadIds);
        pool->threadIds = NULL;
    }

    /*阻塞回收管理者线程的资源*/
    if (pool->managerId)
    {
        pthread_join(pool->managerId, NULL);
    }

    /* 释放锁和条件变量 */
    pthread_mutex_destroy(&(pool->mutexbusy));
    pthread_mutex_destroy(&(pool->mutexPool));
    pthread_cond_destroy(&(pool->notEmpty));
    pthread_cond_destroy(&(pool->notFull));

    return UNKNOWN_ERROR;
}


/* 添加任务 */
int threadPoolAddTask(threadPool_t * pool, void *(*worker_hander)(void * arg), void *arg)
{
    if (pool == NULL)
    {
        return NULL_PTR;
    }
    /*加锁*/
    pthread_mutex_lock(&(pool->mutexPool));
    while(pool->queueSize == pool->queueCapacity && !pool->shutdown)
    {
        /*阻塞生产者线程*/
        pthread_cond_wait(&(pool->notFull), &(pool->mutexPool));
    }
    if (pool->shutdown)
    {
        pthread_mutex_unlock(&(pool->mutexPool));
        exit(-1);
    }
    /* 程序到这个地方一定有位置可以放任务 */
    /* 将任务放到队列的队尾 */
    pool->taskQueue[pool->queueRear].work_handle = worker_hander;
    pool->taskQueue[pool->queueRear].arg = arg;
    /*队尾向后移动*/
    pool->queueRear = (pool->queueRear + 1) % pool->queueCapacity;
    /*任务数加一*/
    pool->queueSize++;
    pthread_mutex_unlock(&(pool->mutexPool));
    /*已经添加任务 向消费者发信号可以消费*/
    pthread_cond_signal(&(pool->notEmpty));

    pthread_mutex_unlock(&(pool->mutexPool));    

    return ON_SUCCESS;
}

/*获取线程池中工作的个数*/
int threadPoolBusyNum(threadPool_t * pool)
{
    pthread_mutex_lock(&(pool->mutexbusy));
    int busyNums = pool->busyThreadNums;
    pthread_mutex_unlock(&(pool->mutexbusy));
    return busyNums;
}

/*获取线程池中活着的线程数*/
int threadPoolLiveNum(threadPool_t * pool)
{
    pthread_mutex_lock(&(pool->mutexPool));
    int liveNums = pool->liveThreadNums;
    pthread_mutex_unlock(&(pool->mutexPool));
    return liveNums;
}

/*线程池的销毁*/
int threadPoolDestroy(threadPool_t * pool)
{
    int ret = 0;
    if (pool == NULL)
    {
        return NULL_PTR;
    }
    /*关闭线程池*/
    pool->shutdown = 1;

    /*阻塞回收管理者线程*/
    pthread_join(pool->managerId, NULL);
    /*唤醒阻塞的消费者线程*/
    for (int idx = 0; idx < pool->liveThreadNums; idx++)
    {
        pthread_cond_signal(&(pool->notEmpty));
    }
    /*释放堆内存*/
    if (pool->taskQueue)
    {
        free(pool->taskQueue);
    }
    if (pool->threadIds)
    {
        free(pool->threadIds);
    }
    free(pool);
    pthread_mutex_destroy(&(pool->mutexPool));
    pthread_mutex_destroy(&(pool->mutexbusy));
    pthread_cond_destroy(&(pool->notEmpty));
    pthread_cond_destroy(&(pool->notFull));
    pool = NULL;
    pool->taskQueue = NULL;
    pool->threadIds = NULL;


    return ret;
}