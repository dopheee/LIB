#ifndef __KEY_H
#define __KEY_H

// 类型重定义
#define KEY_CHAR        char
#define KEY_U8          unsigned char
#define KEY_U16         unsigned short
#define KEY_U32         unsigned int
#define KEY_S8          signed char
#define KEY_S16         signed short
#define KEY_S32         signed int
#define KEY_BOOL        KEY_U8
#define KEY_TRUE        1
#define KEY_FALSE       0

// 按键事件枚举
typedef enum {
    KEY_EVENT_NONE = 0,
    KEY_EVENT_CLICK,
    KEY_EVENT_DOUBLE_CLICK,
    KEY_EVENT_LONG_PRESS
} KeyEvent_t;

// 按键回调函数类型
typedef void (*KeyCallback_t)(KeyEvent_t event);

// 获取按键状态函数类型
typedef KEY_U8 (*GetKeyState_t)(void);

// 获取系统计数函数类型
typedef KEY_U32 (*GetSysTick_t)(void);

// 按键配置结构体
typedef struct {
    GetKeyState_t get_state;     // 获取按键状态函数
    GetSysTick_t get_tick;       // 获取系统计数函数
    KeyCallback_t callback;      // 按键事件回调函数
    KEY_U8 state;                // 按键状态
    KEY_U8 click_count;          // 点击计数
    KEY_U32 last_press_time;     // 上次按下时间
    KEY_U32 last_release_time;   // 上次释放时间
    KEY_U32 press_duration;      // 按下持续时间
    KEY_U32 long_press_time;     // 长按判定时间(ms)
    KEY_U32 double_click_time;   // 双击间隔时间(ms)
    KEY_U32 debounce_time;       // 防抖时间(ms)
    KEY_BOOL long_press_triggered; // 长按触发标志
    KEY_BOOL enabled;            // 使能标志
} KeyConfig_t;

// 函数声明
void key_init(GetSysTick_t get_tick);
KEY_S32 key_register(GetKeyState_t get_state, KeyCallback_t callback);
void key_set_params(int key_id, KEY_U32 debounce_time, KEY_U32 long_press_time, KEY_U32 double_click_time);
void key_poll(void);

#endif

