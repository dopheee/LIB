#ifndef __CMD_H
#define __CMD_H

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

/**
 * @brief 指令回调函数类型
 * @param payload 指令负载数据
 * @param len 负载长度
 */
typedef void (*cmd_callback_t)(const CMD_CHAR *payload, CMD_U16 len);

/**
 * @brief 指令表项结构体
 */
typedef struct {
    const CMD_CHAR *prefix;           ///< 指令前缀
    const CMD_CHAR *suffix;           ///< 指令后缀
    cmd_callback_t callback;          ///< 指令回调函数
} cmd_entry_t;

/**
 * @brief 模板指令表项结构体
 */
typedef struct {
    const CMD_CHAR *template_str;     ///< 模板字符串
    cmd_callback_t callback;          ///< 指令回调函数
} cmd_template_entry_t;

/**
 * @brief 获取当前tick的函数指针类型
 */
typedef CMD_U32 (*cmd_get_tick_t)(void);

/**
 * @brief 起止符对结构体
 */
typedef struct {
    CMD_CHAR start;           ///< 起始字符
    CMD_CHAR end;             ///< 结束字符
    CMD_U8 priority;          ///< 优先级，越大越高
} cmd_delim_pair_t;

/**
 * @brief 初始化指令系统，注册多组起止符及超时回调
 * @param delim_pairs 起止符对数组
 * @param pair_count 数组长度
 * @param get_tick 获取当前tick的函数指针
 * @param timeout_ms 超时时间（毫秒）
 */
void cmd_system_init(const cmd_delim_pair_t *delim_pairs, CMD_U8 pair_count, cmd_get_tick_t get_tick, CMD_U32 timeout_ms);

/**
 * @brief 注册一个前后缀指令处理回调
 * @param prefix 指令前缀
 * @param suffix 指令后缀
 * @param cb 回调函数
 */
void cmd_register_prefix_suffix(const CMD_CHAR *prefix, const CMD_CHAR *suffix, cmd_callback_t cb);

/**
 * @brief 注册一个模板指令处理回调
 * @param template_str 模板字符串，支持*通配符
 * @param cb 回调函数
 */
void cmd_register_template(const CMD_CHAR *template_str, cmd_callback_t cb);

/**
 * @brief 输入一个字符到指令分发器
 * @param c 输入的字符
 */
void cmd_dispatcher_input(CMD_CHAR c);

#endif

