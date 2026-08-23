/**
 * @file thread_pool.h
 * @brief 简易线程池：基于 pthread 的 fork-join 并行执行器
 *        始终编译启用 pthread；线程数由调用方在运行时指定
 *        （num_threads <= 1 时自动退化为串行执行）。
 * @author gmrtest
 * @date 2026
 */

#ifndef GMRTEST_THREAD_POOL_H
#define GMRTEST_THREAD_POOL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 任务函数原型
 * @param arg 任务上下文指针
 * @param task_index 当前任务索引（0 ~ num_tasks-1）
 */
typedef void (*task_func_t)(void *arg, int task_index);

/**
 * @brief 并行执行 num_tasks 个任务
 *        所有任务共享 arg，task_index 区分子任务
 *        - num_threads > 1：使用 num_threads 个工作线程并发执行
 *        - num_threads <= 1：串行执行
 * @param func 任务函数
 * @param arg 共享上下文
 * @param num_tasks 任务数
 * @param num_threads 线程数（<=0 时自动取 gmrtest_recommended_threads()）
 */
void parallel_for(task_func_t func, void *arg, int num_tasks, int num_threads);

/**
 * @brief 获取建议线程数
 *        - 优先读取环境变量 GMRTEST_THREADS
 *        - 否则返回 1（默认单线程）
 */
int gmrtest_recommended_threads(void);

/**
 * @brief 是否支持多线程
 * @return 恒为 1（多线程支持始终编译启用，运行时线程数由 -j 控制）
 */
int gmrtest_multithread_enabled(void);

#ifdef __cplusplus
}
#endif

#endif /* GMRTEST_THREAD_POOL_H */
