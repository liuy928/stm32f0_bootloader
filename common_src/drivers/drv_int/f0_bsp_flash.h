#ifndef __FLASH_H
#define __FLASH_H

#include "stm32f0xx_hal.h"

#define FLASH_ADDR_BASE 0x08000000
#define FLASH_SECTOR_NUM 32
#define FLASH_SECTOR_SIZE 4096ul
#define FLASH_PAGE_NUM 64

extern void bsp_flash_read_n_hword(uint32_t addr, void* destination, uint16_t num);

extern HAL_StatusTypeDef bsp_flash_write_n_hword_nocheck(
    uint32_t addr, void* destination, uint16_t num);
extern HAL_StatusTypeDef bsp_flash_write_n_hword_check(
    uint32_t addr, void* destination, uint16_t num);

extern HAL_StatusTypeDef bsp_flash_erase_npage(uint32_t start_page, uint32_t n_page);

#endif
