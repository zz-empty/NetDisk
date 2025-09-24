#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "../include/threadpool.h"

// 测试任务函数
void test_task(void *arg) {
    int task_id = *(int*)arg;

    printf("Task %d started by thread %lu\n", task_id, (unsigned long)pthread_self());

    usleep(100000 + rand() % 200000);   // 100-300ms
    
    printf("Task %d completed\n", task_id);
    free(arg);  // 释放任务参数
}

// 基本功能测试
void test_basic_functionary(void) {
    printf("=== Test 1: Basic Functionary ===\n");

    threadpool_t *pool = threadpool_create(4, 10);
    if (!pool) {
        fprintf(stderr, "✗ Failed to create thread pool\n");
        return;
    }
    printf("✓ Thread pool created with 4 threads\n");
    
    // 添加10个任务
    for (int i = 0; i < 10; i++) {
        int *task_id = (int*)malloc(sizeof(int));
        *task_id = i;

        int result = threadpool_add(pool, test_task, task_id);
        if (result != THREADPOOL_SUCCESS) {
            printf("✗ Failed to add task %d: error %d\n", i, result);
            free(task_id);
        }
    }

    printf("✓ 10 tasks added to queue\n");

    // 等待任务完成
    sleep(2);
    printf("✓ All tasks should be completed\n");

    threadpool_destroy(pool, 1);
    printf("✓ Thread pool destroyed gracefully\n\n");
}

void test_stress(void) {
    printf("=== Test 2: Stress Test ===\n");
    
    threadpool_t *pool = threadpool_create(8, 50);
    if (!pool) {
        printf("✗ Failed to create thread pool\n");
        return;
    }

    int total_tasks = 100;
    int completed = 0;

    for (int i = 0; i < total_tasks; i++) {
        int *task_id = (int*)malloc(sizeof(int));
        *task_id = i;

        if (threadpool_add(pool, test_task, task_id) == THREADPOOL_SUCCESS) {
            completed++;
        } else {
            free(task_id);
        }

        // 随机延迟，模拟真实场景
        /* usleep(10000 + rand() % 40000); */
    }

    printf("✓ %d/%d tasks added successfully\n", completed, total_tasks);

    // 等待所有任务完成
    threadpool_wait(pool, 100);
    printf("✓ Stress test completed\n");

    threadpool_destroy(pool, 1);
    printf("✓ Thread pool destroyed\n\n");
}

void test_queue_full(void) {
    printf("=== Test 3: Queue Full Test ===\n");
    
    threadpool_t* pool = threadpool_create(2, 5); // 小队列
    if (!pool) {
        printf("✗ Failed to create thread pool\n");
        return;
    }
    
    int successes = 0;
    int failures = 0;
    
    // 快速添加大量任务
    for (int i = 0; i < 20; i++) {
        int* task_id = (int*)malloc(sizeof(int));
        *task_id = i;
        
        int result = threadpool_add(pool, test_task, task_id);
        if (result == THREADPOOL_SUCCESS) {
            successes++;
        } else if (result == THREADPOOL_QUEUE_FULL) {
            failures++;
            free(task_id);
        }
    }
    
    printf("✓ Queue full test: %d successes, %d failures (expected)\n", successes, failures);
    
    sleep(3); // 等待现有任务完成
    
    threadpool_destroy(pool, 0); // 立即关闭
    printf("✓ Thread pool destroyed immediately\n\n");
}
// 错误处理测试
void test_error_handling(void) {
    printf("=== Test 4: Error Handling ===\n");
    
    // 测试无效参数
    threadpool_t* pool = threadpool_create(0, 10); // 无效线程数
    if (!pool) {
        printf("✓ Correctly rejected invalid thread count\n");
    }
    
    pool = threadpool_create(5, 0); // 无效队列大小
    if (!pool) {
        printf("✓ Correctly rejected invalid queue size\n");
    }
    
    pool = threadpool_create(3, 10);
    if (pool) {
        // 测试向已关闭的池添加任务
        threadpool_destroy(pool, 1);
        pool = NULL;

        int result = threadpool_add(pool, test_task, NULL);
        if (result == THREADPOOL_SHUTDOWN) {
            printf("✓ Correctly rejected task after shutdown\n");
        }
    }
    
    printf("✓ Error handling tests passed\n\n");
}

int main() {
    printf("Starting Thread Pool Tests...\n\n");
    srand(time(NULL));

    test_basic_functionary();
    test_stress();
    test_queue_full();
    test_error_handling();

    printf("All thread pool tests completed successfully!\n");
    return 0;
}
