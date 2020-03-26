#ifndef __BSP_SPI_H
#define __BSP_SPI_H

#include "system_platform_config.h"
#include "stm32f0xx_ll_spi.h"

extern void bsp_spi_config(SPI_TypeDef* SPIx);
extern uint8_t bsp_spi_exchange_data(SPI_TypeDef* SPIx, uint8_t dat);

#endif
