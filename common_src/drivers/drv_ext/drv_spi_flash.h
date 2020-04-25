#ifndef __DRV_SPI_FLASH_H
#define __DRV_SPI_FLASH_H

#include "system_platform_config.h"

// 定义flash属性，起始地址、块大小、扇区大小、页大小；
#define W25QXX_BASED_ADDRESS   (0x000000)
#define W25QXX_BLOCK_SIZE      (64 * 1024ul) ul
#define W25QXX_SECTOR_SIZE     (4 * 1024ul) // 0x001000;
#define W25QXX_PAGE_SIZE       (256ul)

#define W25X_BUSY_MASK        0x01

#define W25X_PAGE_WR          0x02
#define W25X_RD_DATA          0x03
#define W25X_RD_STATUS        0x05
#define W25X_WR_ENABLE        0x06
#define W25X_SECTOR_ERASE     0x20
#define W25X_CHIP_ERASE       0xc7

#define W25Q_CS(a)   if(a) \
                      LL_GPIO_SetOutputPin(DRV_W25_CS_PORT, DRV_W25_CS_PIN); \
                     else \
                      LL_GPIO_ResetOutputPin(DRV_W25_CS_PORT, DRV_W25_CS_PIN)

extern void v_drv_w25q_init(void);

extern void v_drv_w25q_sector_erase(uint32_t ul_sector_addr);
extern void v_drv_w25q_chip_erase(void);

extern void v_drv_w25q_read(uint8_t* puc_des, uint32_t ul_addr, uint16_t us_len);

extern void v_drv_w25q_page_write_no_check(uint8_t* puc_srs, uint32_t ul_addr, uint16_t us_len);
extern void v_drv_w25q_brust_write_no_check(uint8_t* puc_srs, uint32_t ul_addr, uint16_t us_len);

extern void v_drv_w25q_brust_write_check(uint8_t* puc_srs, uint32_t ul_addr, uint16_t us_len);

extern uint32_t ulDrvSPIFlashGetJedecID(void);

#endif
