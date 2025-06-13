#include "sdio_app.h"
#include <stdio.h>
#include <string.h>

#define SDIO_BLK_SIZE (512U)
#define SDIO_SWAP_BUF_SIZE (16 * 1024U)

uint8_t sdio_swap_buf[SDIO_SWAP_BUF_SIZE] __ALIGNED(4);

int sd_disk_write(uint8_t *buf, uint32_t blk_addr, uint32_t blk_cnt)
{
  sd_error_status_type errorcode = 0;
  if (blk_cnt == 0)
    return -1;

  do
  {
#ifdef SD_USE_DMA
    if ((uint32_t)buf % 4)
    {
      /* buf addr not aligned 4 bytes */
      if (blk_cnt == 1)
      {
        /* move data to aligned buffer */
        memcpy(sdio_swap_buf, buf, SDIO_BLK_SIZE);

        /* write single block data */
        if (SD_OK != sd_block_write(sdio_swap_buf, blk_addr * SDIO_BLK_SIZE,
                                    SDIO_BLK_SIZE))
        {
          errorcode = 1;
          break;
        }
      }
      else
      {
        /* multiple blocks write */
        uint32_t CopyTimes = blk_cnt / (SDIO_SWAP_BUF_SIZE / SDIO_BLK_SIZE);
        uint32_t LeftBlocks = blk_cnt % (SDIO_SWAP_BUF_SIZE / SDIO_BLK_SIZE);

        while (CopyTimes--)
        {
          /* copy data to swap buffer */
          memcpy(sdio_swap_buf, buf, SDIO_SWAP_BUF_SIZE);

          if (SD_OK != sd_mult_blocks_write(
                           sdio_swap_buf, blk_addr * SDIO_BLK_SIZE,
                           SDIO_BLK_SIZE, (SDIO_SWAP_BUF_SIZE / SDIO_BLK_SIZE)))
          {
            errorcode = 2;
            break;
          }

          buf += SDIO_SWAP_BUF_SIZE;
          blk_cnt -= (SDIO_SWAP_BUF_SIZE / SDIO_BLK_SIZE);
        }
        if (errorcode)
          break;

        if (LeftBlocks)
        {
          memcpy(sdio_swap_buf, buf, LeftBlocks * SDIO_BLK_SIZE);

          if (LeftBlocks == 1)
          {
            if (SD_OK != sd_block_write(sdio_swap_buf, blk_addr * SDIO_BLK_SIZE,
                                        SDIO_BLK_SIZE))
            {
              errorcode = 3;
              break;
            }
          }
          else
          {
            if (SD_OK != sd_mult_blocks_write(sdio_swap_buf,
                                              blk_addr * SDIO_BLK_SIZE,
                                              SDIO_BLK_SIZE, LeftBlocks))
            {
              errorcode = 4;
              break;
            }
          }
        }
      }
    }
    else
    {
#endif
      /* buffer aligned */
      if (blk_cnt == 1)
      {
        if (SD_OK !=
            sd_block_write(buf, blk_addr * SDIO_BLK_SIZE, SDIO_BLK_SIZE))
        {
          errorcode = 5;
          break;
        }
      }
      else
      {
        if (SD_OK != sd_mult_blocks_write(buf, blk_addr * SDIO_BLK_SIZE,
                                          SDIO_BLK_SIZE, blk_cnt))
        {
          errorcode = 6;
          break;
        }
      }
#ifdef SD_USE_DMA
    }
#endif

  } while (0);

  if (errorcode)
    printf("sd_disk_write error code: %d\r\n", errorcode);
  return errorcode;
}

int sd_disk_read(uint8_t *buf, uint32_t blk_addr, uint32_t blk_cnt)
{
  sd_error_status_type errorcode = 0;
  if (blk_cnt == 0)
    return -1;

  do
  {
#ifdef SD_USE_DMA
    if ((uint32_t)buf % 4)
    {
      /* buf addr not aligned 4 bytes */
      if (blk_cnt == 1)
      {
        /* read single block data */
        if (SD_OK != sd_block_read(sdio_swap_buf, blk_addr * SDIO_BLK_SIZE,
                                   SDIO_BLK_SIZE))
        {
          errorcode = 1;
          break;
        }

        /* move data to aligned buffer */
        memcpy(buf, sdio_swap_buf, SDIO_BLK_SIZE);
      }
      else
      {
        /* multiple blocks write */
        uint32_t CopyTimes = blk_cnt / (SDIO_SWAP_BUF_SIZE / SDIO_BLK_SIZE);
        uint32_t LeftBlocks = blk_cnt % (SDIO_SWAP_BUF_SIZE / SDIO_BLK_SIZE);

        while (CopyTimes--)
        {
          if (SD_OK != sd_mult_blocks_read(
                           sdio_swap_buf, blk_addr * SDIO_BLK_SIZE,
                           SDIO_BLK_SIZE, (SDIO_SWAP_BUF_SIZE / SDIO_BLK_SIZE)))
          {
            errorcode = 2;
            break;
          }

          /* copy data to swap buffer */
          memcpy(buf, sdio_swap_buf, SDIO_SWAP_BUF_SIZE);

          buf += SDIO_SWAP_BUF_SIZE;
          blk_cnt -= (SDIO_SWAP_BUF_SIZE / SDIO_BLK_SIZE);
        }
        if (errorcode)
          break;

        if (LeftBlocks)
        {
          if (LeftBlocks == 1)
          {
            if (SD_OK != sd_block_read(sdio_swap_buf, blk_addr * SDIO_BLK_SIZE,
                                       SDIO_BLK_SIZE))
            {
              errorcode = 3;
              break;
            }
          }
          else
          {
            if (SD_OK != sd_mult_blocks_read(sdio_swap_buf,
                                             blk_addr * SDIO_BLK_SIZE,
                                             SDIO_BLK_SIZE, LeftBlocks))
            {
              errorcode = 4;
              break;
            }
          }

          memcpy(sdio_swap_buf, buf, LeftBlocks * SDIO_BLK_SIZE);
        }
      }
    }
    else
    {
#endif
      /* buffer aligned */
      if (blk_cnt == 1)
      {
        if (SD_OK !=
            sd_block_read(buf, blk_addr * SDIO_BLK_SIZE, SDIO_BLK_SIZE))
        {
          errorcode = 5;
          break;
        }
      }
      else
      {
        if (SD_OK != sd_mult_blocks_read(buf, blk_addr * SDIO_BLK_SIZE,
                                         SDIO_BLK_SIZE, blk_cnt))
        {
          errorcode = 6;
          break;
        }
      }
#ifdef SD_USE_DMA
    }
#endif

  } while (0);

  if (errorcode)
    printf("sd_disk_read error code: %d\r\n", errorcode);
  return errorcode;
}