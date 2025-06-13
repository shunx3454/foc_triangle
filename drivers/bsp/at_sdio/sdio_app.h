#ifndef __SDIO_APP_H_
#define __SDIO_APP_H_

#include "at32_sdio.h"

#define SD_USE_DMA

int sd_disk_write(uint8_t *buf, uint32_t blk_addr, uint32_t blk_cnt);
int sd_disk_read(uint8_t *buf, uint32_t blk_addr, uint32_t blk_cnt);

#endif