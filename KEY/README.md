# 🎯 KEY 按键处理模块

`key.c` 和 `key.h` 提供了一个**灵活高效**的按键检测与事件处理机制，支持单击、双击和长按事件，适用于嵌入式系统、MCU、单片机等场景。

---

## ✨ 功能简介

- 🎮 **多按键支持**：最多支持8个独立按键
- 🚀 **事件检测**：支持单击、双击和长按事件
- ⚙️ **参数可配置**：每个按键的防抖时间、长按判定时间和双击间隔时间均可独立配置
- 🔄 **回调机制**：通过回调函数通知按键事件，使用灵活
- 🛡️ **防抖处理**：内置防抖功能，避免信号干扰

---

## 📝 使用方法

1. **初始化模块**：调用 `key_init` 函数初始化按键模块，传入获取系统时间的函数
2. **注册按键**：调用 `key_register` 函数注册按键，传入获取按键状态的函数和事件回调函数
3. **配置参数**：可选，调用 `key_set_params` 函数设置按键的防抖时间、长按时间和双击时间
4. **轮询检测**：在主循环中调用 `key_poll` 函数进行按键检测

---

## 📚 API 参考

### 🔧 函数

```c
// 初始化按键模块
void key_init(GetSysTick_t get_tick);

// 注册按键
KEY_S32 key_register(GetKeyState_t get_state, KeyCallback_t callback);

// 设置按键参数
void key_set_params(int key_id, KEY_U32 debounce_time, KEY_U32 long_press_time, KEY_U32 double_click_time);

// 按键轮询函数
void key_poll(void);
```

### 📊 数据类型

```c
// 按键事件枚举
typedef enum {
    KEY_EVENT_NONE = 0,        // 无事件
    KEY_EVENT_CLICK,           // 单击事件
    KEY_EVENT_DOUBLE_CLICK,    // 双击事件
    KEY_EVENT_LONG_PRESS       // 长按事件
} KeyEvent_t;

// 按键回调函数类型
typedef void (*KeyCallback_t)(KeyEvent_t event);

// 获取按键状态函数类型
typedef KEY_U8 (*GetKeyState_t)(void);

// 获取系统计数函数类型
typedef KEY_U32 (*GetSysTick_t)(void);
```

---

## 💻 示例代码

```c
// 初始化系统tick函数
extern uint32_t GetSystemTick(void);

// 按键状态获取函数
KEY_U8 GetKey1State(void)
{
    // 返回按键1的状态，1表示按下，0表示未按下
    return HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET ? 1 : 0;
}

// 按键事件回调函数
void Key1Callback(KeyEvent_t event)
{
    switch(event)
    {
        case KEY_EVENT_CLICK:
            // 处理单击事件
            printf("单击事件触发\n");
            break;
        case KEY_EVENT_DOUBLE_CLICK:
            // 处理双击事件
            printf("双击事件触发\n");
            break;
        case KEY_EVENT_LONG_PRESS:
            // 处理长按事件
            printf("长按事件触发\n");
            break;
        default:
            break;
    }
}

// 初始化
void main(void)
{
    // 初始化按键模块
    key_init(GetSystemTick);

    // 注册按键
    int key_id = key_register(GetKey1State, Key1Callback);
    if(key_id >= 0)
    {
        // 设置按键参数（可选）
        key_set_params(key_id, 50, 1000, 300);  // 防抖50ms，长按1000ms，双击间隔300ms
    }

    // 主循环
    while(1)
    {
        // 轮询按键
        key_poll();

        // 其他任务
        // ...
    }
}
```

---

## 📌 注意事项

- 确保在主循环中定期调用 `key_poll` 函数，以保证按键事件被及时检测
- 长按事件触发后，单击计数会被重置
- 双击事件只有在规定的时间间隔内发生两次单击才会触发
- 按键状态获取函数应返回0表示未按下，1表示按下