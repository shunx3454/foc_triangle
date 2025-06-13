#include "uart.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char          uart_buffer[UART_DMA_BUFFER_SIZE];
uint16_t      uart_buffer_read_index = 0;
__IO uint32_t uart_dma_cplt_flag     = 0;

static uint16_t static_cmd_index       = 0;
static uint16_t static_cmd_param_index = 0;
char            cmd_parse_buffer[CMD_BUFFER_SIZE];
int             cmd_param_buffer[CMD_PARAM_BUFFER_SIZE];
cmd_t           cmd_table[CMD_TABLE_SIZE];

void uart_debug_init(uint32_t baudrate)
{
    gpio_init_type gpio_init_struct;
    dma_init_type  dma_init_struct;
    gpio_default_para_init(&gpio_init_struct);

#if defined(__GNUC__) && !defined(__clang__)
    setvbuf(stdout, NULL, _IONBF, 0);
#endif

    /* gpio------------------------------------------------------------------------ */
    /* configure the TX pin */
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
    gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode           = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pins           = GPIO_PINS_9;
    gpio_init_struct.gpio_pull           = GPIO_PULL_NONE;
    gpio_init(GPIOA, &gpio_init_struct);

    /* configure the RX pin */
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
    gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode           = GPIO_MODE_INPUT;
    gpio_init_struct.gpio_pins           = GPIO_PINS_10;
    gpio_init_struct.gpio_pull           = GPIO_PULL_NONE;
    gpio_init(GPIOA, &gpio_init_struct);

    /* dma------------------------------------------------------------------------ */
    /* flexible function enable */
    dma_flexible_config(DMA1, FLEX_CHANNEL2, DMA_FLEXIBLE_UART1_RX);
    dma_default_para_init(&dma_init_struct);
    dma_reset(DMA1_CHANNEL2);
    dma_init_struct.buffer_size           = UART_DMA_BUFFER_SIZE;
    dma_init_struct.direction             = DMA_DIR_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_base_addr      = (uint32_t) uart_buffer;
    dma_init_struct.memory_data_width     = DMA_MEMORY_DATA_WIDTH_BYTE;
    dma_init_struct.memory_inc_enable     = TRUE;
    dma_init_struct.peripheral_base_addr  = (uint32_t) &(USART1->dt);
    dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
    dma_init_struct.peripheral_inc_enable = FALSE;
    dma_init_struct.priority              = DMA_PRIORITY_MEDIUM;
    dma_init_struct.loop_mode_enable      = TRUE;
    dma_init(DMA1_CHANNEL2, &dma_init_struct);

    /* enable dma transfer complete interrupt */
    dma_flag_clear(DMA1_FDT2_FLAG);
    dma_interrupt_enable(DMA1_CHANNEL2, DMA_FDT_INT, TRUE);

    dma_channel_enable(DMA1_CHANNEL2, TRUE);

    /* usart------------------------------------------------------------------------ */
    /* configure uart param */
    usart_init(USART1, baudrate, USART_DATA_8BITS, USART_STOP_1_BIT);
    usart_transmitter_enable(USART1, TRUE);
    usart_receiver_enable(USART1, TRUE);
    usart_parity_selection_config(USART1, USART_PARITY_NONE);
    usart_hardware_flow_control_set(USART1, USART_HARDWARE_FLOW_NONE);

    /* dma request */
    usart_dma_receiver_enable(USART1, TRUE);

    usart_enable(USART1, TRUE);
}

void DMA1_Channel2_IRQHandler(void)
{
    if (dma_interrupt_flag_get(DMA1_FDT2_FLAG) != RESET)
    {
        dma_flag_clear(DMA1_FDT2_FLAG);
        printf("uart dma cplt\r\n");
        uart_dma_cplt_flag = 1;
    }
}

// index position no data
int32_t uart_get_dma_index(void)
{
    return UART_DMA_BUFFER_SIZE - dma_data_number_get(DMA1_CHANNEL2);
}

int32_t find_char(uint8_t* buffer, uint8_t c, uint16_t offset, uint16_t size, uint16_t buffer_wrap)
{
    for (uint16_t i = 0; i < size; i++)
    {
        if (buffer[(offset + i) % buffer_wrap] == c)
        {
            return i;
        }
    }
    return -1;
}

int32_t find_string(uint8_t* buffer, uint8_t* str, uint16_t offset, uint16_t size, uint16_t buffer_wrap)
{
    for (uint16_t i = 0; i < size; i++)
    {
        if (buffer[(offset + i) % buffer_wrap] == str[0])
        {
            for (uint16_t j = 1; j < strlen((char*) str); j++)
            {
                if (i + j >= size)
                {
                    return -1;
                }
                if (buffer[(offset + i + j) % buffer_wrap] != str[j])
                {
                    break;
                }
                if (j == strlen((char*) str) - 1)
                {
                    return i;
                }
            }
        }
    }
    return -1;
}

int32_t uart_cmd_find(void)
{
    uint16_t end_index = uart_get_dma_index();
    int16_t  offset    = 0;
    uint16_t left_size = 0;

    // if dma transfer complete
    if (uart_dma_cplt_flag == 1)
    {
        uart_dma_cplt_flag = 0;
        if (end_index > uart_buffer_read_index)
        {
            uart_buffer_read_index = end_index;
        }
        left_size = UART_DMA_BUFFER_SIZE - uart_buffer_read_index + end_index;
    }
    else
    {
        left_size = end_index - uart_buffer_read_index;
    }

    offset = find_string(uart_buffer, (uint8_t*) "AT", uart_buffer_read_index, left_size, UART_DMA_BUFFER_SIZE);
    if (offset < 0)
    {
        return -1;
    }
    else
    {
        uart_buffer_read_index += offset;
        uart_buffer_read_index %= UART_DMA_BUFFER_SIZE;
        left_size -= offset;
        offset = find_string(uart_buffer, (uint8_t*) "\r\n", uart_buffer_read_index, left_size, UART_DMA_BUFFER_SIZE);

        if (offset < 0)
        {
            return -1;
        }
        else
        {
            if (offset <= CMD_BUFFER_SIZE - 1)
            {
                for (uint16_t i = 0; i < offset; i++)
                {
                    cmd_parse_buffer[i] = uart_buffer[(uart_buffer_read_index + i) % UART_DMA_BUFFER_SIZE];
                }
                cmd_parse_buffer[offset] = '\0';
                uart_buffer_read_index += offset + 2;
                uart_buffer_read_index %= UART_DMA_BUFFER_SIZE;
                return 0;
            }
            else
            {
                uart_buffer_read_index += (offset + 2);
                uart_buffer_read_index %= UART_DMA_BUFFER_SIZE;
                return -1;
            }
        }
    }
}

void uart_cmd_parse(void)
{
    if (uart_cmd_find() == 0)
    {
        // printf("%s\r\n", cmd_parse_buffer);

        if (cmd_parse_buffer[2] == '\0')
        {
            printf("AT OK\r\n");
            return;
        }
        else if (cmd_parse_buffer[2] == '+')
        {
            char*    str              = NULL;
            char*    ptr              = NULL;
            int*     param_ptr        = NULL;
            uint16_t parsed_param_num = 0;

            for (uint16_t i = 0; i < static_cmd_index; i++)
            {
                str = strstr(&cmd_parse_buffer[3], (char*) cmd_table[i].key);
                if (str != NULL)
                {
                    if (*str == cmd_parse_buffer[3])
                    {
                        str += strlen((char*) cmd_table[i].key);
                        if (*str == '\0')
                        {
                            cmd_table[i].cmd_func(NULL);
                            return;
                        }
                        else if (*str == '=')
                        {
                            str++;
                            param_ptr = cmd_table[i].int_param;

                            // 整型数解析
                            while (1)
                            {
                                int data = strtol(str, &ptr, 10);
                                if (ptr == str)
                                {
                                    printf("AT ERROR\r\n");
                                    return;
                                }
                                else if (*ptr == ',')
                                {
                                    parsed_param_num++;
                                    *param_ptr = data;
                                    param_ptr++;
                                    str = ptr + 1;
                                    continue;
                                }
                                else if (*ptr == '\0')
                                {
                                    *param_ptr = data;
                                    parsed_param_num++;
                                    if (parsed_param_num == cmd_table[i].int_param_num)
                                    {
                                        cmd_table[i].cmd_func(cmd_table[i].int_param);
                                        // printf("AT OK\r\n");
                                        return;
                                    }
                                    else
                                    {
                                        printf("AT ERROR\r\n");
                                        return;
                                    }
                                }
                                else
                                {
                                    printf("AT ERROR\r\n");
                                    return;
                                }
                            }
                        }
                        else if (*str == '?')
                        {
                            printf("%s ?\r\n", cmd_table[i].key);
                            return;
                        }
                        else
                        {
                            printf("AT ERROR\r\n");
                            return;
                        }
                    }
                    else
                    {
                        printf("AT ERROR\r\n");
                        return;
                    }
                }
            }
            printf("AT ERROR\r\n");
        }
        else if (cmd_parse_buffer[2] == '?')
        {
            if (cmd_parse_buffer[3] == '\0')
            {
                printf("AT CMD ...\r\n");
                return;
            }
            else
            {
                printf("AT ERROR\r\n");
                return;
            }
        }
        else
        {
            printf("AT ERROR\r\n");
            return;
        }
    }
}

void uart_cmd_register(uint8_t* cmd_key, uint8_t max_param_num, void (*cmd_func)(void*))
{
    if (static_cmd_index >= CMD_TABLE_SIZE)
    {
        return;
    }
    if (static_cmd_param_index >= CMD_PARAM_BUFFER_SIZE)
    {
        return;
    }

    for (uint16_t i = 0; i < strlen((char*) cmd_key) + 1; i++)
    {
        if (i == CMD_KEY_SIZE - 1)
        {
            cmd_table[static_cmd_index].key[i] = '\0';
            break;
        }
        cmd_table[static_cmd_index].key[i] = cmd_key[i];
    }
    cmd_table[static_cmd_index].int_param_num = max_param_num;
    cmd_table[static_cmd_index].cmd_func      = cmd_func;
    cmd_table[static_cmd_index].int_param     = &cmd_param_buffer[static_cmd_param_index];

    static_cmd_param_index += max_param_num;
    static_cmd_index++;
}
