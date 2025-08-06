# 🚀 CMD 指令解析模块

`cmd.c` 和 `cmd.h` 提供了一个**灵活高效**的指令解析系统，支持前后缀指令识别、模板指令匹配和参数化回调，适用于嵌入式系统、串口通信和命令行交互场景。

---

## ✨ 功能简介

- 📋 **多指令支持**：最多支持8个前后缀指令和4个模板指令
- 🔍 **模式匹配**：支持精确前缀+后缀匹配和带通配符的模板匹配
- 🎯 **参数化回调**：支持带负载数据的回调函数
- ⚡ **多组起止符**：支持最多4组自定义指令起止符
- 🕒 **超时处理**：内置接收超时机制
- 🛠️ **灵活配置**：支持动态注册指令和自定义超时时间

---

## 📝 使用方法

1. **初始化模块**：调用 `cmd_system_init` 函数初始化指令系统，传入起止符对、获取系统时间的函数和超时时间
2. **注册指令**：调用 `cmd_register_prefix_suffix` 或 `cmd_register_template` 函数注册指令处理回调
3. **处理输入**：在主循环中调用 `cmd_dispatcher_input` 函数处理字符输入

---

## 📚 API 参考

### 🔧 函数

```c
// 注册一个前后缀指令处理回调
void cmd_register_prefix_suffix(const CMD_CHAR *prefix, const CMD_CHAR *suffix, cmd_callback_t cb);

// 注册一个模板指令处理回调
void cmd_register_template(const CMD_CHAR *template_str, cmd_callback_t cb);

// 初始化指令系统，注册多组起止符及超时回调
void cmd_system_init(const cmd_delim_pair_t *delim_pairs, CMD_U8 pair_count, cmd_get_tick_t get_tick, CMD_U32 timeout_ms);

// 字符输入处理函数
void cmd_dispatcher_input(CMD_CHAR c);
```

### 📊 数据类型

```c
// 指令回调函数类型
typedef void (*cmd_callback_t)(const CMD_CHAR *payload, CMD_U16 len);

// 获取当前tick的函数指针类型
typedef CMD_U32 (*cmd_get_tick_t)(void);

// 指令表项结构体
typedef struct {
    const CMD_CHAR *prefix;           ///< 指令前缀
    const CMD_CHAR *suffix;           ///< 指令后缀
    cmd_callback_t callback;          ///< 指令回调函数
} cmd_entry_t;

// 模板指令表项结构体
typedef struct {
    const CMD_CHAR *template_str;     ///< 模板字符串
    cmd_callback_t callback;          ///< 指令回调函数
} cmd_template_entry_t;

// 起止符对结构体
typedef struct {
    CMD_CHAR start;           ///< 起始字符
    CMD_CHAR end;             ///< 结束字符
} cmd_delim_pair_t;

// 类型重定义
#define CMD_CHAR        char
#define CMD_U8          unsigned char
#define CMD_U16         unsigned short
#define CMD_U32         unsigned int
#define CMD_S8          signed char
#define CMD_S16         signed short
#define CMD_S32         signed int
#define CMD_BOOL        CMD_U8
#define CMD_TRUE        1
#define CMD_FALSE       0
```

---

## 💻 示例代码

```c
// 初始化系统tick函数
extern uint32_t GetSystemTick(void);

// 前后缀指令回调函数
void LedCmdCallback(const CMD_CHAR *payload, CMD_U16 len)
{
    // 处理LED指令
    printf("LED指令执行，负载: %.*s\n", len, payload);
    if (len > 0) {
        // 根据负载控制LED
    }
}

// 模板指令回调函数
void SetCmdCallback(const CMD_CHAR *payload, CMD_U16 len)
{
    // 处理设置指令
    printf("设置指令执行，参数: %.*s\n", len, payload);
}

int main(void)
{
    // 定义起止符对
    cmd_delim_pair_t delim_pairs[] = {
        {'$', '!'},  // 指令格式: $指令内容!
        {'#', ';'}
    };

    // 初始化指令系统
    cmd_system_init(delim_pairs, 2, GetSystemTick, 500);

    // 注册前后缀指令
    cmd_register_prefix_suffix("LED", "CTRL", LedCmdCallback);

    // 注册模板指令 (支持*通配符)
    cmd_register_template("SET_*", SetCmdCallback);

    // 主循环
    while (1) {
        // 处理输入字符
        // 这里假设从串口读取字符
        CMD_CHAR c = getchar();
        // 调用指令处理函数
        cmd_dispatcher_input(c);
    }
}
```

---

## 📱 指令触发示例及输出

### 前后缀指令示例

```c
// 输入指令: $LED1CTRL!
// 输出结果: 
// LED指令执行，负载: 1

// 输入指令: $LED2CTRL!
// 输出结果:
// LED指令执行，负载: 2

```

### 模板指令示例

```c
// 输入指令: #SET_TIMEOUT=1000;
// 输出结果:
// 设置指令执行，参数: TIMEOUT=1000

// 输入指令: #SET_MODE=AUTO;
// 输出结果:
// 设置指令执行，参数: MODE=AUTO
```

---

## ⚠️ 注意事项

- 确保在调用指令处理函数前已正确初始化指令系统
- 指令前缀和后缀应尽量简短且唯一，避免冲突
- 模板指令中的通配符`*`仅支持简单匹配，不支持复杂正则表达式
- 接收超时时间应根据通信波特率和指令长度合理设置