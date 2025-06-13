#ifndef __BSP_SPI_H_
#define __BSP_SPI_H_

#include "at32f403a_407.h"


#ifdef __cplusplus
extern "C"
{
#endif


void spi_configuration(spi_type* SPIx);
void spi_cs_high(spi_type* SPIx);
void spi_cs_low(spi_type* SPIx);


#ifdef __cplusplus
}
#endif

#endif