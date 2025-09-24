#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "../include/threadpool.h"

// 线程工作函数
static void *threadpool_worker(void *arg);

// 创建线程池
threadpool_t *threadpool_create(int thread_count, int queue_size) {
    if (thread_count <= 0 || thread_count > 100 || queue_size <= 0 || queue_size > 100) {
        return NULL;
    }

    threadpool_t *pool = (threadpool_t*)malloc(sizeof(threadpool_t));
    if (!pool) {
        return NULL;
    }

    // 初始化线程池成员
    pool->thread_count = 0;
    pool->queue_size = 0;
    pool->max_queue_size = queue_size;
    pool->shutdown = 0;
    pool->started = 0;
    pool->task_queue_head = NULL;
    pool->task_queue_tail = NULL;

    // 创建线程数组
    pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * thread_count);
    if (!pool->threads) {
        free(pool);
        return NULL;
    }

    // 初始化锁和条件便利
    if (pthread_mutex_init(&pool->lock, NULL) != 0) {
        free(pool->threads);
        free(pool);
        return NULL;
    }

    if (pthread_cond_init(&pool->notify, NULL) != 0) {
        pthread_mutex_destroy(&pool->lock);
        free(pool->threads);
        free(pool);
        return NULL;
    }

    // 创建工作线程
    for (int i = 0; i < thread_count; ++i) {
        if (pthread_create(pool->threads + i, NULL, threadpool_worker, (void*)pool) != 0) {
            threadpool_destroy(pool, 1);
            return NULL;
        }

        pool->thread_count++;
        pool->started++;
    }

    return pool;
}

// 线程工作函数
static void *threadpool_worker(void *arg) {
    threadpool_t *pool = (threadpool_t*)arg;
    task_t *task;

    while (1) {
        // 加锁拿任务
        pthread_mutex_lock(&pool->lock);
        
        // 用while唤醒，防止伪唤醒
        while (pool->queue_size == 0 && !pool->shutdown) {
            pthread_cond_wait(&pool->notify, &pool->lock);
        }

        // 是否退出
        if (pool->shutdown) {
            pool->started--;
            pthread_mutex_unlock(&pool->lock);
            break;
        }

        // 取出一个任务
        task = pool->task_queue_head;
        if (!task) {
            pthread_mutex_unlock(&pool->lock);
            continue;
        }

        pool->task_queue_head = pool->task_queue_head->next;
        pool->queue_size--;

        if (pool->queue_size == 0) {
            pool->task_queue_tail = NULL;
        }

        pthread_mutex_unlock(&pool->lock);

        // 执行任务
        if (task) {
            task->function(task->arg);
            free(task);
        }
    }

    pthread_exit(NULL);
    return NULL;
}

int threadpool_add(threadpool_t *pool, task_func_t function, void *arg) {
    if (!pool || !function) {
        return THREADPOOL_INVALID;
    }

    if (pthread_mutex_lock(&pool->lock) != 0) {
        return THREADPOOL_LOCK_FAILED;
    }

    // 线程池已关闭
    if (pool->shutdown) {
        pthread_mutex_unlock(&pool->lock);
        return THREADPOOL_SHUTDOWN;
    }

    // 队列已满
    if (pool->queue_size == pool->max_queue_size) {
        pthread_mutex_unlock(&pool->lock);
        return THREADPOOL_QUEUE_FULL;
    }

    // 创建任务
    task_t *task = (task_t*)malloc(sizeof(task_t));
    if (!task) {
        pthread_mutex_unlock(&pool->lock);
        return THREADPOOL_INVALID;  
    } 

    task->function = function;
    task->arg = arg;
    task->next = NULL;

    // 加入队列
    if (pool->task_queue_tail) {
        pool->task_queue_tail->next = task;
    } else {
        pool->task_queue_head = task;
    }

    pool->task_queue_tail = task;
    pool->queue_size++;

    // 通知等待的工作线程
    if (pthread_cond_signal(&pool->notify) != 0) {
        pthread_mutex_unlock(&pool->lock);
        return THREADPOOL_LOCK_FAILED;
    }

    pthread_mutex_unlock(&pool->lock);
    return THREADPOOL_SUCCESS;
}

// 销毁线程池
int threadpool_destroy(threadpool_t *pool, int graceful) {
    if (!pool) {
        return THREADPOOL_INVALID;
    }

    if (pthread_mutex_lock(&pool->lock) != 0) {
        return THREADPOOL_LOCK_FAILED;
    }

    // 避免重复关闭
    if (pool->shutdown) {
        pthread_mutex_unlock(&pool->lock);
        return THREADPOOL_SHUTDOWN;
    }

    pool->shutdown = (graceful) ? 1 : 2;

    // 唤醒所有线程
    if (pthread_cond_broadcast(&pool->notify) != 0 || 
        pthread_mutex_unlock(&pool->lock) != 0) {
        return THREADPOOL_SHUTDOWN;
    }

    // 等待所有线程退出
    for (int i = 0; i < pool->thread_count; i++) {
        if (pthread_join(pool->threads[i], NULL) != 0) {
            return THREADPOOL_LOCK_FAILED;
        }
    }

    // 清理资源
    int status = threadpool_free(pool);
    pool = NULL;
    return status;
}

// 释放线程池资源
int threadpool_free(threadpool_t *pool) {
    if (!pool || pool->started > 0) {
        return THREADPOOL_INVALID;
    }

    // 释放未完成的任务
    if (pool->threads) {
        free(pool->threads);

        task_t *task;
        while (pool->task_queue_head) {
            task = pool->task_queue_head;
            pool->task_queue_head = pool->task_queue_head->next;
            free(task);
        }

        pthread_mutex_lock(&pool->lock);
        pthread_mutex_destroy(&pool->lock);
        pthread_cond_destroy(&pool->notify);
    }

    // 释放线程池
    free(pool);
    return THREADPOOL_SUCCESS;
}

// 等待线程池完成任务
int threadpool_wait(threadpool_t *pool, int timeout_seconds) {
    if (!pool) {
        return THREADPOOL_INVALID;
    }

    int waited = 0;
    while (pool->queue_size > 0 && waited < timeout_seconds) {
        sleep(1);
        waited++;
    }

    return (pool->queue_size == 0) ? THREADPOOL_SUCCESS : THREADPOOL_INVALID;
}

// 获取当前队列大小
int threadpool_get_queue_size(threadpool_t *pool) {
    if (!pool) {
        return THREADPOOL_INVALID;
    }
    pthread_mutex_lock(&pool->lock);
    int size = pool->queue_size;
    pthread_mutex_unlock(&pool->lock);
    return size;
}

// 获取线程数量
int threadpool_get_thread_count(threadpool_t *pool) {
    if (!pool) {
        return THREADPOOL_INVALID;
    }

    pthread_mutex_lock(&pool->lock);
    int count = pool->thread_count;
    pthread_mutex_unlock(&pool->lock);
    return count;
}
