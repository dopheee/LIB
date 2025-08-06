#ifndef __SCHED_H
#define __SCHED_H

// 类型重定义
#define SCHED_CHAR        char
#define SCHED_U8          unsigned char
#define SCHED_U16         unsigned short
#define SCHED_U32         unsigned int
#define SCHED_S8          signed char
#define SCHED_S16         signed short
#define SCHED_S32         signed int
#define SCHED_BOOL        SCHED_U8
#define SCHED_TRUE        1
#define SCHED_FALSE       0

/**
 * @brief 定时任务回调函数类型（带参数）
 * @param param 用户自定义参数指针
 */
typedef void (*sched_callback_t)(void *param);

/**
 * @brief 获取当前tick的函数指针类型
 */
typedef SCHED_U32 (*sched_get_tick_t)(void);

/**
 * @brief 定时任务结构体
 */
typedef struct
{
    SCHED_U32 interval_ms;      /* 任务周期（ms） */
    SCHED_U32 last_tick;        /* 上次触发tick */
    sched_callback_t callback;  /* 回调函数 */
    void *param;                /* 回调参数 */
    SCHED_U8 enabled;           /* 使能标志 */
} sched_task_t;

/**
 * @brief 初始化定时任务调度系统
 * @param get_tick 获取当前tick的函数指针
 */
void sched_init(sched_get_tick_t get_tick);

/**
 * @brief 注册一个定时任务
 * @param interval 任务周期（ms）
 * @param cb 回调函数
 * @param param 回调参数
 * @return 成功返回任务编号，失败返回-1
 */
int sched_register(SCHED_U32 interval, sched_callback_t cb, void *param);

/**
 * @brief 轮询调度任务，需在主循环中周期调用
 */
void sched_poll(void);

#endif
