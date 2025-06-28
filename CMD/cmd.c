#include "cmd.h"

#define CMD_MAX_ENTRIES 8
#define CMD_BUF_SIZE    128

static cmd_entry_t cmd_table[CMD_MAX_ENTRIES];
static unsigned char cmd_count = 0;

static char rx_buf[CMD_BUF_SIZE];
static unsigned short rx_len = 0;
static const cmd_entry_t *current_cmd = (void*)0;

static const char *cmd_start = "$";
static const char *cmd_end   = "!";

/**
 * @brief 注册一个指令处理回调
 * @param prefix 指令前缀
 * @param suffix 指令后缀
 * @param cb 回调函数
 */
void cmd_register(const char *prefix, const char *suffix, cmd_callback_t cb)
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
 * @brief 初始化指令系统，设置起始和结束字符
 * @param start 起始字符
 * @param end 结束字符
 */
void cmd_system_init(const char *start, const char *end)
{
    cmd_start = start;
    cmd_end = end;
    rx_len = 0;
}

/**
 * @brief 判断字符是否为起始字符
 * @param c 输入字符
 * @return 1-是起始字符，0-不是
 */
static int is_start_char(char c)
{
    const char *p;
    for (p = cmd_start; *p; ++p)
    {
        if (c == *p)
            return 1;
    }
    return 0;
}

/**
 * @brief 判断字符是否为结束字符
 * @param c 输入字符
 * @return 1-是结束字符，0-不是
 */
static int is_end_char(char c)
{
    const char *p;
    for (p = cmd_end; *p; ++p)
    {
        if (c == *p)
            return 1;
    }
    return 0;
}

/**
 * @brief 输入一个字符到指令分发器
 * @param c 输入的字符
 */
void cmd_dispatcher_input(char c)
{
    unsigned char i;
    unsigned int prefix_len, suffix_len, payload_len;
    char payload[CMD_BUF_SIZE];

    if (is_start_char(c))
    {
        rx_len = 0;
        rx_buf[rx_len++] = c;
        rx_buf[rx_len] = '\0';
        return;
    }

    if (rx_len == 0)
        return;

    if (rx_len < CMD_BUF_SIZE - 1)
    {
        rx_buf[rx_len++] = c;
        rx_buf[rx_len] = '\0';
    }
    else
    {
        rx_len = 0;
        return;
    }

    if (is_end_char(c))
    {
        for ( i = 0; i < cmd_count; i++)
        {
            prefix_len = strlen(cmd_table[i].prefix);
            suffix_len = strlen(cmd_table[i].suffix);

            if (rx_len >= 2 + prefix_len + suffix_len)
            {
                if (strncmp(rx_buf + 1, cmd_table[i].prefix, prefix_len) == 0 &&
                    strncmp(rx_buf + rx_len - 1 - suffix_len, cmd_table[i].suffix, suffix_len) == 0)
                {
                    payload_len = rx_len - 2 - prefix_len - suffix_len;
                    if (payload_len > 0)
                    {
                        memcpy(payload, rx_buf + 1 + prefix_len, payload_len);
                        payload[payload_len] = '\0';
                    }
                    else
                    {
                        payload[0] = '\0';
                    }
                    cmd_table[i].callback(payload, payload_len);
                    break;
                }
            }
        }
        rx_len = 0;
    }
}
