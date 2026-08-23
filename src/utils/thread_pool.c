/**
 * @file thread_pool.c
 * @brief 基于 pthread 的简易并行 for 实现
 *        - 始终编译 pthread 版本
 *        - num_threads <= 1 时串行执行
 *        - 线程数由调用方指定（命令行 -j / 环境变量 GMRTEST_THREADS）
 * @author gmrtest
 * @date 2026
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "thread_pool.h"

int gmrtest_multithread_enabled(void)
{
    /* 多线程支持始终编译启用，运行时线程数由 -j 控制 */
    return 1;
}

int gmrtest_recommended_threads(void)
{
    /* 优先读取环境变量 GMRTEST_THREADS（兼容旧接口） */
    const char *env = getenv("GMRTEST_THREADS");
    if (env != NULL && *env != '\0') {
        int n = atoi(env);
        if (n > 0) return n;
    }
    /* 未指定时默认单线程 */
    return 1;
}

typedef struct {
    task_func_t   func;
    void         *arg;
    int           num_tasks;
    /* 原子计数器：worker 通过 __sync_fetch_and_add 抢任务 */
    volatile int  next_index;
} parallel_ctx_t;

static void *worker_main(void *p)
{
    parallel_ctx_t *ctx = (parallel_ctx_t *)p;
    for (;;) {
        int idx = __sync_fetch_and_add(&ctx->next_index, 1);
        if (idx >= ctx->num_tasks) break;
        ctx->func(ctx->arg, idx);
    }
    return NULL;
}

void parallel_for(task_func_t func, void *arg, int num_tasks, int num_threads)
{
    parallel_ctx_t ctx;
    pthread_t *threads = NULL;
    int i, t;

    if (func == NULL || num_tasks <= 0) {
        return;
    }

    if (num_threads <= 0) {
        num_threads = gmrtest_recommended_threads();
    }
    if (num_threads > num_tasks) {
        num_threads = num_tasks;
    }
    if (num_threads <= 1) {
        /* 单线程：串行执行 */
        for (i = 0; i < num_tasks; i++) {
            func(arg, i);
        }
        return;
    }

    ctx.func = func;
    ctx.arg = arg;
    ctx.num_tasks = num_tasks;
    ctx.next_index = 0;

    threads = (pthread_t *)calloc((size_t)num_threads, sizeof(pthread_t));
    if (threads == NULL) {
        /* 分配失败：退化为串行 */
        for (i = 0; i < num_tasks; i++) {
            func(arg, i);
        }
        return;
    }

    for (t = 0; t < num_threads; t++) {
        if (pthread_create(&threads[t], NULL, worker_main, &ctx) != 0) {
            /* 创建失败：等待已启动的线程，剩余任务由主线程处理 */
            int started = t;
            for (i = 0; i < started; i++) {
                pthread_join(threads[i], NULL);
            }
            for (;;) {
                int idx = __sync_fetch_and_add(&ctx.next_index, 1);
                if (idx >= ctx.num_tasks) break;
                ctx.func(ctx.arg, idx);
            }
            free(threads);
            return;
        }
    }

    /* 主线程也参与抢任务，避免空转 */
    for (;;) {
        int idx = __sync_fetch_and_add(&ctx.next_index, 1);
        if (idx >= ctx.num_tasks) break;
        ctx.func(ctx.arg, idx);
    }

    for (t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);
    }
    free(threads);
}
