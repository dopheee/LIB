#include "cmd.h"
#include <stddef.h>

#define CMD_MAX_ENTRIES      8
#define CMD_MAX_TEMPLATES    4
#define CMD_BUF_SIZE         128
#define CMD_MAX_DELIM_PAIRS  4

static cmd_entry_t cmd_table[CMD_MAX_ENTRIES];
static CMD_U8 cmd_count = 0;

static cmd_template_entry_t cmd_template_table[CMD_MAX_TEMPLATES];
static CMD_U8 cmd_template_count = 0;

static CMD_CHAR rx_buf[CMD_BUF_SIZE];
static CMD_U16 rx_len = 0;
static const cmd_entry_t *current_cmd = (void*)0;

static const CMD_CHAR *cmd_start = "$";
static const CMD_CHAR *cmd_end   = "!";

static cmd_delim_pair_t cmd_delim_pairs[CMD_MAX_DELIM_PAIRS];
static CMD_U8 cmd_delim_pair_count = 0;
static cmd_get_tick_t cmd_get_tick = 0;
static CMD_U32 cmd_timeout_ms = 100;

static CMD_S32 active_delim_idx = -1; // 当前激活的起止符对索引
static CMD_U32 last_tick = 0;

/**
 * @brief 注册一个前后缀指令处理回调
 * @param prefix 指令前缀
 * @param suffix 指令后缀
 * @param cb 回调函数
 */
void cmd_register_prefix_suffix(const CMD_CHAR *prefix, const CMD_CHAR *suffix, cmd_callback_t cb)
{
    if (cmd_count < CMD_MAX_ENTRIES)
    {
        cmd_table[cmd_count].prefix = prefix;
        cmd_table[cmd_count].suffix = suffix;
        cmd_table[cmd_count].callback = cb;
        cmd_count++;
    }
}

/**
 * @brief 注册一个模板指令处理回调
 * @param template_str 模板字符串，支持*通配符
 * @param cb 回调函数
 */
void cmd_register_template(const CMD_CHAR *template_str, cmd_callback_t cb)
{
    if (cmd_template_count < CMD_MAX_TEMPLATES)
    {
        cmd_template_table[cmd_template_count].template_str = template_str;
        cmd_template_table[cmd_template_count].callback = cb;
        cmd_template_count++;
    }
}

/**
 * @brief 初始化指令系统，注册多组起止符及超时回调
 * @param delim_pairs 起止符对数组
 * @param pair_count 数组长度
 * @param get_tick 获取当前tick的函数指针
 * @param timeout_ms 超时时间（毫秒）
 */
void cmd_system_init(const cmd_delim_pair_t *delim_pairs, CMD_U8 pair_count, cmd_get_tick_t get_tick, CMD_U32 timeout_ms)
{
    CMD_U8 i;
    cmd_delim_pair_count = (pair_count > CMD_MAX_DELIM_PAIRS) ? CMD_MAX_DELIM_PAIRS : pair_count;
    for (i = 0; i < cmd_delim_pair_count; i++) {
        cmd_delim_pairs[i] = delim_pairs[i];
    }
    cmd_get_tick = get_tick;
    cmd_timeout_ms = timeout_ms;
    rx_len = 0;
    active_delim_idx = -1;
    last_tick = 0;
}

/**
 * @brief 判断字符是否为起始字符
 * @param c 输入字符
 * @return 1-是起始字符，0-不是
 */
static CMD_U8 is_start_char(CMD_CHAR c)
{
    for (const CMD_CHAR *p = cmd_start; *p; ++p)
    {
        if (c == *p)
            return CMD_TRUE;
    }
    return CMD_FALSE;
}

/**
 * @brief 判断字符是否为结束字符
 * @param c 输入字符
 * @return 1-是结束字符，0-不是
 */
static CMD_U8 is_end_char(CMD_CHAR c)
{
    for (const CMD_CHAR *p = cmd_end; *p; ++p)
    {
        if (c == *p)
            return CMD_TRUE;
    }
    return CMD_FALSE;
}

/**
 * @brief 模板匹配函数，支持*通配符
 * @param buf 输入指令
 * @param template_str 模板字符串
 * @return 1-匹配，0-不匹配
 */
static int match_template(const CMD_CHAR *buf, const CMD_CHAR *template_str)
{
    while (*template_str && *buf)
    {
        if (*template_str != '*' && *template_str != *buf)
            return 0;
        template_str++;
        buf++;
    }
    return (*template_str == '\0' && *buf == '\0');
}

/**
 * @brief 输入一个字符到指令分发器
 * @param c 输入的字符
 */
void cmd_dispatcher_input(CMD_CHAR c)
{
    CMD_U8 i;
    CMD_U32 prefix_len, suffix_len, payload_len;
    CMD_CHAR payload[CMD_BUF_SIZE]; 

    // 超时处理
    if (active_delim_idx >= 0 && cmd_get_tick && (cmd_get_tick() - last_tick > cmd_timeout_ms)) {
        rx_len = 0;
        active_delim_idx = -1;
    }

    // 记录时间
    if (cmd_get_tick) last_tick = cmd_get_tick();

    // 未激活时，查找优先级最高的起始符
    if (active_delim_idx < 0) {
        int max_priority = -1, idx = -1;
        for (i = 0; i < cmd_delim_pair_count; i++) {
            if (c == cmd_delim_pairs[i].start && cmd_delim_pairs[i].priority > max_priority) {
                max_priority = cmd_delim_pairs[i].priority;
                idx = i;
            }
        }
        if (idx >= 0) {
            active_delim_idx = idx;
            rx_len = 0;
            rx_buf[rx_len++] = c;
            rx_buf[rx_len] = '\0';
            return;
        }
        return; // 非起始符直接丢弃
    }

    // 已激活，收集字符
    if (rx_len < CMD_BUF_SIZE - 1) {
        rx_buf[rx_len++] = c;
        rx_buf[rx_len] = '\0';
    } else {
        rx_len = 0;
        active_delim_idx = -1;
        return;
    }

    // 检查是否为对应的结束符
    if (c == cmd_delim_pairs[active_delim_idx].end) {
        // 1. 先模板匹配
        for (i = 0; i < cmd_template_count; i++) {
            if (match_template(rx_buf, cmd_template_table[i].template_str)) {
                payload_len = rx_len - 2;
                if (payload_len > 0) {
                    memcpy(payload, rx_buf + 1, payload_len);
                    payload[payload_len] = '\0';
                } else {
                    payload[0] = '\0';
                }
                cmd_template_table[i].callback(payload, payload_len);
                rx_len = 0;
                active_delim_idx = -1;
                return;
            }
        }
        // 2. 前后缀匹配
        for (i = 0; i < cmd_count; i++) {
            prefix_len = strlen(cmd_table[i].prefix);
            suffix_len = strlen(cmd_table[i].suffix);
            if (rx_len >= 2 + prefix_len + suffix_len) {
                if (strncmp(rx_buf + 1, cmd_table[i].prefix, prefix_len) == 0 &&
                    strncmp(rx_buf + rx_len - 1 - suffix_len, cmd_table[i].suffix, suffix_len) == 0) {
                    payload_len = rx_len - 2 - prefix_len - suffix_len;
                    if (payload_len > 0) {
                        memcpy(payload, rx_buf + 1 + prefix_len, payload_len);
                        payload[payload_len] = '\0';
                    } else {
                        payload[0] = '\0';
                    }
                    cmd_table[i].callback(payload, payload_len);
                    break;
                }
            }
        }
        rx_len = 0;
        active_delim_idx = -1;
    }
}
