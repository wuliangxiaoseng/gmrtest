/**
 * @file thread_pool.c
 * @brief 基于 pthread 的简易并行 for 实现
 *        - USE_PTHREAD 启用时：原子计数器 + N 个 worker thread
 *        - 未启用时：串行执行
 * @author gmrtest
 * @date 2026
 */

#include <stdio.h>
#include <stdlib.h>
#include "thread_pool.h"

#ifdef USE_PTHREAD
#  include <pthread.h>
#  include <unistd.h>
#endif

int gmrtest_multithread_enabled(void)
{
#ifdef USE_PTHREAD
    return 1;
#else
    return 0;
#endif
}

int gmrtest_recommended_threads(void)
{
#ifdef USE_PTHREAD
    const char *env = getenv("GMRTEST_THREADS");
    if (env != NULL && *env != '\0') {
        int n = atoi(env);
        if (n > 0) return n;
    }
    {
        long n = sysconf(_SC_NPROCESSORS_ONLN);
        if (n > 0) return (int)n;
    }
    return 4;
#else
    return 1;
#endif
}

#ifdef USE_PTHREAD

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
        /* 退化为串行 */
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

#else  /* !USE_PTHREAD —— 串行回退 */

void parallel_for(task_func_t func, void *arg, int num_tasks, int num_threads)
{
    int i;
    (void)num_threads;
    if (func == NULL || num_tasks <= 0) return;
    for (i = 0; i < num_tasks; i++) {
        func(arg, i);
    }
}

#endif
