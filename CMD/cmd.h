#ifndef __CMD_H
#define __CMD_H

/**
 * @brief 指令回调函数类型
 * @param payload 指令负载数据
 * @param len 负载长度
 */
typedef void (*cmd_callback_t)(const char *payload, unsigned short len);

/**
 * @brief 指令表项结构体
 */
typedef struct {
    const char *prefix;           ///< 指令前缀
    const char *suffix;           ///< 指令后缀
    cmd_callback_t callback;      ///< 指令回调函数
} cmd_entry_t;

/**
 * @brief 初始化指令系统，设置起始和结束字符
 * @param start 起始字符
 * @param end 结束字符
 */
void cmd_system_init(const char *start, const char *end);

/**
 * @brief 注册一个指令处理回调
 * @param prefix 指令前缀
 * @param suffix 指令后缀
 * @param cb 回调函数
 */
void cmd_register(const char *prefix, const char *suffix, cmd_callback_t cb);

/**
 * @brief 输入一个字符到指令分发器
 * @param c 输入的字符
 */
void cmd_dispatcher_input(char c);

#endif

