#include "oled.h"
#include "i2c_application.h"
#include "string.h"
#include "system.h"
#include <stdio.h>

#define OLED_I2C_ADDR 0x78
static i2c_handle_type oled_i2c;
static uint8_t         oled_gram[8][128];

extern const unsigned char asc2_0806[95][6];
extern const unsigned char asc2_1206[95][12];
extern const unsigned char asc2_1608[95][16];
extern const unsigned char asc2_2412[95][36];

int8_t oled_write_data(uint8_t* pdata, uint16_t len)
{
    i2c_status_type i2c_res = i2c_memory_write(&oled_i2c, I2C_MEM_ADDR_WIDIH_8, OLED_I2C_ADDR, 0x40, pdata, len, 1000);
    return (i2c_res == I2C_OK) ? 0 : -1;
}
int8_t oled_write_cmd(uint8_t cmd)
{
    i2c_status_type i2c_res = i2c_memory_write(&oled_i2c, I2C_MEM_ADDR_WIDIH_8, OLED_I2C_ADDR, 0x00, &cmd, 1, 1000);
    return (i2c_res == I2C_OK) ? 0 : -1;
}

void oled_init(void)
{
    oled_i2c.i2cx = I2C2;
    i2c_config(&oled_i2c);

    oled_write_cmd(0xAE); //--turn off oled panel
    oled_write_cmd(0x00); //---set low column address
    oled_write_cmd(0x10); //---set high column address
    oled_write_cmd(0x40); //--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
    oled_write_cmd(0x81); //--set contrast control register
    oled_write_cmd(0xCF); // Set SEG Output Current Brightness
    oled_write_cmd(0xA1); //--Set SEG/Column Mapping
    oled_write_cmd(0xC8); // Set COM/Row Scan Direction
    oled_write_cmd(0xA6); //--set normal display
    oled_write_cmd(0xA8); //--set multiplex ratio(1 to 64)
    oled_write_cmd(0x3f); //--1/64 duty
    oled_write_cmd(0xD3); //-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
    oled_write_cmd(0x00); //-not offset
    oled_write_cmd(0xd5); //--set display clock divide ratio/oscillator frequency
    oled_write_cmd(0x80); //--set divide ratio, Set Clock as 100 Frames/Sec
    oled_write_cmd(0xD9); //--set pre-charge period
    oled_write_cmd(0xF1); // Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    oled_write_cmd(0xDA); //--set com pins hardware configuration
    oled_write_cmd(0x12);
    oled_write_cmd(0xDB); //--set vcomh
    oled_write_cmd(0x30); // Set VCOM Deselect Level
    oled_write_cmd(0x20); //-Set Page Addressing Mode (0x00/0x01/0x02)
    oled_write_cmd(0x02); //
    oled_write_cmd(0x8D); //--set Charge Pump enable/disable
    oled_write_cmd(0x14); //--set(0x10) disable
    oled_write_cmd(0xAF);

    oled_clear();
    oled_fresh();
}

void oled_fresh(void)
{
    for (int i = 0; i < 8; i++)
    {
        oled_write_cmd(0xb0 + i);
        oled_write_cmd(0x00);
        oled_write_cmd(0x10);
        oled_write_data(&oled_gram[i][0], 128);
    }
}
void oled_clear(void)
{
    uint8_t* ptr = &oled_gram[0][0];
    for (int i = 0; i < 128 * 8; i++)
    {
        ptr[i] = 0;
    }
}

void oled_display_on(void)
{
    oled_write_cmd(0x8d);
    oled_write_cmd(0x14);
    oled_write_cmd(0xaf);
}

void oled_display_off(void)
{
    oled_write_cmd(0x8d);
    oled_write_cmd(0x10);
    oled_write_cmd(0xae);
}

void oled_draw_point(uint8_t li, uint8_t row, uint8_t state)
{
    uint8_t line, in;

    if (li >= 64 || row >= 128)
        return;

    line = li / 8;
    in   = li % 8;

    if (state)
    {
        oled_gram[line][row] |= 1 << in;
    }
    else
    {
        oled_gram[line][row] &= ~(1 << in);
    }
}

void oled_draw_8points(uint8_t line, uint8_t row, uint8_t points)
{
    if (line >= 8 || row >= 128)
        return;
    oled_gram[line][row] = points;
}

int8_t oled_show_char(uint8_t li, uint8_t row, uint8_t font, unsigned char ch)
{
    uint8_t *ptr, chrow, chline;

    if (li >= 64 || row >= 128 || (font != 8 && font != 12 && font != 16 && font != 24))
        return -1;

    ch -= ' ';

    switch (font)
    {
        case 8:
            ptr    = (uint8_t*) &asc2_0806[ch][0];
            chline = 1;
            chrow  = 6;
            break;
        case 12:
            ptr    = (uint8_t*) &asc2_1206[ch][0];
            chline = 2;
            chrow  = 6;
            break;
        case 16:
            ptr    = (uint8_t*) &asc2_1608[ch][0];
            chline = 2;
            chrow  = 8;
            break;
        case 24:
            ptr    = (uint8_t*) &asc2_2412[ch][0];
            chline = 3;
            chrow  = 12;
            break;
        default:
            break;
    }

    if (li + font > 64 || row + chrow > 128)
        return -2;

    for (int line = 0; line < chline; line++)
    {

        if (font == 12 && line == chline - 1)
        {
            for (int j = 0; j < chrow; j++)
            {
                for (int k = 0; k < 4; k++)
                    oled_draw_point(li + line * 8 + k, row + j, (ptr[line * chrow + j] >> k) & 1);
            }
        }
        else if (li % 8 == 0)
        {
            for (int j = 0; j < chrow; j++)
            {
                oled_draw_8points(li / 8 + line, row + j, ptr[line * chrow + j]);
            }
        }
        else
        {
            for (int j = 0; j < chrow; j++)
            {
                for (int k = 0; k < 8; k++)
                    oled_draw_point(li + line * 8 + k, row + j, (ptr[line * chrow + j] >> k) & 1);
            }
        }
    }
    return 0;
}

int8_t oled_show_string(uint8_t li, uint8_t row, uint8_t font, unsigned char* str)
{
    uint8_t chrow, chline, i, j;

    switch (font)
    {
        case 8:
            chline = 1;
            chrow  = 6;
            break;
        case 12:
            chline = 2;
            chrow  = 6;
            break;
        case 16:
            chline = 2;
            chrow  = 8;
            break;
        case 24:
            chline = 3;
            chrow  = 12;
            break;
        default:
            return -3;
            break;
    }
    i = li;
    j = row;
    while (*str != '\0')
    {
        if (j >= 128)
        {
            j = 0;
            i += chline * 8;
            if (i >= 64)
                return -1;
        }
        oled_show_char(i, j, font, *str);
        j += chrow;
        str++;
    }
    return 0;
}

void oled_show_num(uint8_t li, uint8_t row, uint8_t font, int32_t num, uint8_t len)
{
    uint8_t ch[12];
    sprintf(ch, "%ld", num);
    ch[len] = 0;
    oled_show_string(li, row, font, ch);
}

uint32_t power(uint32_t m, uint8_t n)
{
    uint32_t result = 1;
    while (n--)
        result *= m;
    return result;
}

void oled_show_float(uint8_t li, uint8_t row, uint8_t font, float num, uint8_t integer_len, uint8_t decimal_len)
{
    uint8_t  str[25];
    uint8_t* ptr;
    uint8_t  nega = 0;
    int32_t  integer_part;
    int32_t  decimal_part;

    if (num < 0)
    {
        num  = -num;
        nega = 1;
    }
    integer_part = (int) num;
    decimal_part  = (int) ((num - (float) integer_part) * power(10, decimal_len));
    memset(str, 0, sizeof(str));

    // 整数
    ptr = &str[0];
    if (nega)
    {
        *ptr = '-';
        ptr++;
    }

    while (integer_len--)
    {
        *ptr++ = '0' + ((integer_part / power(10, integer_len)) % 10);
    }
    // 小数
    *ptr = '.';
    ptr++;
    while (decimal_len--)
    {
        *ptr++ = '0' + ((decimal_part / power(10, decimal_len)) % 10);
    }

    oled_show_string(li, row, font, str);
}