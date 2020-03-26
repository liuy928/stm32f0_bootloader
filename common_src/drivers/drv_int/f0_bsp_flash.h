#ifndef __FLASH_H
#define __FLASH_H

#include "stm32f0xx_hal.h"

#define FLASH_ADDR_BASE       0x08000000
#define FLASH_SECTOR_NUM      32
#define FLASH_SECTOR_SIZE	    4096ul

#if defined(STM32F030xC)
  #define FLASH_PAGE_NUM        127
#elif defined(STM32F072xB)
  #define FLASH_PAGE_NUM        32
#endif //

extern void bsp_flash_read_n_hword(uint32_t addr,
                                   void *destination,
                                   uint16_t num);

extern HAL_StatusTypeDef bsp_flash_write_n_hword_nocheck(uint32_t addr,
                                                         void *destination,
                                                         uint16_t num);
extern HAL_StatusTypeDef FLASH_write_n_hword_check(uint32_t addr,
                                                   void *destination,
                                                   uint16_t num) ;

extern HAL_StatusTypeDef vBspFlashEraseNPage(uint32_t start_page, uint32_t ul_num);

#endif
