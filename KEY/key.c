#include "key.h"
#include <stddef.h>

#define MAX_KEYS 8  // 最大支持8个按键
static KeyConfig_t key_list[MAX_KEYS];
static GetSysTick_t sys_tick_func = NULL;

/**
 * @brief 按键模块初始化
 * @param get_tick 获取系统计数函数
 */
void key_init(GetSysTick_t get_tick)
{
    if (get_tick == NULL)
        return;
    
    sys_tick_func = get_tick;
    // 初始化按键列表
    for (KEY_U8 i = 0; i < MAX_KEYS; i++)
    {
        key_list[i].enabled = KEY_FALSE;
        key_list[i].callback = NULL;
        key_list[i].get_state = NULL;
        key_list[i].get_tick = get_tick;
        
        // 设置默认参数
        key_list[i].debounce_time = 50;
        key_list[i].long_press_time = 1000;
        key_list[i].double_click_time = 300;
    }
}

/**
 * @brief 注册按键
 * @param get_state 获取按键状态函数
 * @param callback 按键事件回调函数
 * @return 成功返回按键ID，失败返回-1
 */
KEY_S32 key_register(GetKeyState_t get_state, KeyCallback_t callback)
{
    if (get_state == NULL || callback == NULL || sys_tick_func == NULL)
        return -1;
    
    for (KEY_U8 i = 0; i < MAX_KEYS; i++)
    {
        if (!key_list[i].enabled)
        {
            key_list[i].get_state = get_state;
            key_list[i].callback = callback;
            key_list[i].enabled = KEY_TRUE;
            key_list[i].state = 0;
            key_list[i].click_count = 0;
            key_list[i].last_press_time = 0;
            key_list[i].last_release_time = 0;
            key_list[i].press_duration = 0;
            key_list[i].long_press_triggered = KEY_FALSE;
            return i;
        }
    }
    return -1; // 没有空闲按键
}

/**
 * @brief 设置按键参数
 * @param key_id 按键ID
 * @param debounce_time 防抖时间(ms)
 * @param long_press_time 长按判定时间(ms)
 * @param double_click_time 双击间隔时间(ms)
 */
void key_set_params(int key_id, KEY_U32 debounce_time, KEY_U32 long_press_time, KEY_U32 double_click_time)
{
    if (key_id < 0 || key_id >= MAX_KEYS || !key_list[key_id].enabled)
        return;
    
    key_list[key_id].debounce_time = debounce_time;
    key_list[key_id].long_press_time = long_press_time;
    key_list[key_id].double_click_time = double_click_time;
}

/**
 * @brief 按键处理函数(内部使用)
 */
static void key_process(KeyConfig_t *key)
{
    if (key == NULL || key->get_state == NULL || key->get_tick == NULL)
        return;

    static KEY_U8 key_state = 0;
    static KEY_U8 last_key_state = 0;
    KEY_U32 current_tick = key->get_tick();

    // 获取当前按键状态 (0: 未按下, 1: 按下)
    key_state = key->get_state();

    // 状态变化检测
    if (key_state != last_key_state)
    {
        if (key_state == 1)  // 按键按下
        {
            key->last_press_time = current_tick;
            key->long_press_triggered = KEY_FALSE;  // 重置长按触发标志
        }
        else  // 按键释放
        {
            key->last_release_time = current_tick;
            key->press_duration = current_tick - key->last_press_time;

            // 防抖处理
            if (key->press_duration >= key->debounce_time)
            {
                key->click_count++;
            }
        }
    }

    // 长按检测
    if (key_state == 1 && !key->long_press_triggered && 
        (current_tick - key->last_press_time) >= key->long_press_time)
    {
        if (key->callback != NULL)
        {
            key->callback(KEY_EVENT_LONG_PRESS);
        }
        key->long_press_triggered = KEY_TRUE;  // 标记长按已触发
        key->click_count = 0;
    }

    // 双击检测
    if (key_state == 0 && key->click_count > 0)
    {
        if ((current_tick - key->last_release_time) >= key->double_click_time)
        {
            if (key->callback != NULL)
            {
                if (key->click_count == 1)
                {
                    key->callback(KEY_EVENT_CLICK);
                }
                else if (key->click_count == 2)
                {
                    key->callback(KEY_EVENT_DOUBLE_CLICK);
                }
            }
            key->click_count = 0;
        }
    }

    last_key_state = key_state;
}

/**
 * @brief 按键轮询函数
 * 需在主循环中周期调用
 */
void key_poll(void)
{
    if (sys_tick_func == NULL)
        return;
    
    for (KEY_U8 i = 0; i < MAX_KEYS; i++)
    {
        if (key_list[i].enabled)
        {
            key_process(&key_list[i]);
        }
    }
}

