#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <pthread.h>
/* #include <stddef.h> */

// 任务回调函数类型
typedef void (*task_func_t)(void *arg);

// 任务结构体
typedef struct task {
    task_func_t function;       // 任务函数
    void *arg;                  // 任务参数
    struct task *next;
} task_t;

// 线程池结构体
typedef struct threadpool {
    pthread_mutex_t lock;       // 线程锁
    pthread_cond_t notify;      // 条件变量
    pthread_t *threads;         // 线程数组
    task_t *task_queue_head;     // 任务队列头
    task_t *task_queue_tail;     // 任务队列尾
    int thread_count;           // 线程数量
    int queue_size;             // 当前队列大小
    int max_queue_size;         // 最大队列大小
    int shutdown;               // 关闭标志
    int started;                // 已经启动的线程数
} threadpool_t;

// 错误码
typedef enum {
    THREADPOOL_SUCCESS = 0,
    THREADPOOL_INVALID = -1,
    THREADPOOL_LOCK_FAILED = -2,
    THREADPOOL_QUEUE_FULL = -3,
    THREADPOOL_SHUTDOWN = -4,
    THREADPOOL_THREAD_FAILURE = -5
} threadpool_error_t;

threadpool_t *threadpool_create(int thread_count, int queue_size);
int threadpool_add(threadpool_t *pool, task_func_t function, void *arg);
int threadpool_destroy(threadpool_t *pool, int graceful);
int threadpool_free(threadpool_t *pool);
int threadpool_wait(threadpool_t *pool, int timeout_seconds);
int threadpool_get_queue_size(threadpool_t *pool);
int threadpool_get_thread_count(threadpool_t *pool);

#endif  // THREADPOOL_H
