#ifndef __BSP_UART_H_
#define __BSP_UART_H_

#include "at32f403a_407.h"

#ifdef __cplusplus
extern "C"
{
#endif
#define CMD_PARAM_BUFFER_SIZE 128 // 所有命令的参数存储缓冲区大小
#define CMD_TABLE_SIZE 16         // 所有命令的结构体缓冲区大小
#define CMD_BUFFER_SIZE 32        // 命令字符串最大长度
#define CMD_KEY_SIZE 8            // 命令关键字大小

    typedef struct
    {
        char     key[CMD_KEY_SIZE];
        uint16_t int_param_num;
        void (*cmd_func)(void*);
        int* int_param;
    } cmd_t;

#define UART_DMA_BUFFER_SIZE 512 // UART RX DMA最大接收长度

    extern char uart_buffer[UART_DMA_BUFFER_SIZE];

    void    uart_debug_init(uint32_t baudrate);
    int32_t uart_cmd_find(void);
    void    uart_cmd_parse(void);
    void    uart_cmd_register(uint8_t* cmd_key, uint8_t max_param_num, void (*cmd_func)(void*));
    void    DMA1_Channel2_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif
