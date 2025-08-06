#include "sched.h"
#include <stddef.h>

#define MAX_SCHED_TASKS 8

static sched_task_t sched_tasks[MAX_SCHED_TASKS];
static sched_get_tick_t sched_get_tick = NULL;

/**
 * @brief 初始化定时任务调度系统
 * @param get_tick 获取当前tick的函数指针
 */
void sched_init(sched_get_tick_t get_tick)
{
    int i;
    sched_get_tick = get_tick;
    for (i = 0; i < MAX_SCHED_TASKS; i++)
    {
        sched_tasks[i].enabled = 0;
    }
}

/**
 * @brief 注册一个定时任务
 * @param interval 任务周期（ms）
 * @param cb 回调函数
 * @param param 回调参数
 * @return 成功返回任务编号，失败返回-1
 */
int sched_register(SCHED_U32 interval, sched_callback_t cb, void *param)
{
    SCHED_S32 i;
    SCHED_U32 now_tick = sched_get_tick ? sched_get_tick() : 0;
    for (i = 0; i < MAX_SCHED_TASKS; i++)
    {
        if (!sched_tasks[i].enabled)
        {
            sched_tasks[i].interval_ms = interval;
            sched_tasks[i].last_tick = now_tick;
            sched_tasks[i].callback = cb;
            sched_tasks[i].param = param;
            sched_tasks[i].enabled = 1;
            return i;
        }
    }
    return -1;
}

/**
 * @brief 轮询调度任务，需在主循环中周期调用
 */
void sched_poll(void)
{
    SCHED_S32 i;
    SCHED_U32 now_tick = sched_get_tick ? sched_get_tick() : 0;
    SCHED_BOOL normal_task_executed = SCHED_FALSE;

    // 先执行所有非0周期且到期的任务
    for (i = 0; i < MAX_SCHED_TASKS; i++)
    {
        if (sched_tasks[i].enabled && sched_tasks[i].interval_ms > 0)
        {
            if ((now_tick - sched_tasks[i].last_tick) >= sched_tasks[i].interval_ms)
            {
                sched_tasks[i].last_tick = now_tick;
                if (sched_tasks[i].callback)
                {
                    sched_tasks[i].callback(sched_tasks[i].param);
                }
                normal_task_executed = 1;
            }
        }
    }

    // 只有当本轮没有其它任务需要执行时，才执行所有周期为0的任务
    if (!normal_task_executed)
    {
        for (i = 0; i < MAX_SCHED_TASKS; i++)
        {
            if (sched_tasks[i].enabled && sched_tasks[i].interval_ms == 0)
            {
                if (sched_tasks[i].callback)
                {
                    sched_tasks[i].callback(sched_tasks[i].param);
                }
            }
        }
    }
}

