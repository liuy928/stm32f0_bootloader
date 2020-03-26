#ifndef __DRV_OPT3001_H
#define __DRV_OPT3001_H

#include "system_platform_config.h"

/* Slave address */
#define OPT3001_I2C_ADDRESS             (0x44)

/* Register addresses */
#define REG_RESULT                      (0x00)
#define REG_CONFIGURATION               (0x01)
#define REG_LOW_LIMIT                   (0x02)
#define REG_HIGH_LIMIT                  (0x03)

#define REG_MANUFACTURER_ID             (0x7E)
#define REG_DEVICE_ID                   (0x7F)

/* Register values */
#define MANUFACTURER_ID                 (0x5449)  // TI
#define DEVICE_ID                       (0x3001)  // Opt 3001
#define CONFIG_ENABLE                   (0xc410) // 0xC410   - 100 ms, continuous               
#define CONFIG_DISABLE                  (0xc010) // 0xC010   - 100 ms, shutdown

extern void vDrvOpt3001Init(void);

extern uint8_t ucDrvOpt3001WriteByte(uint8_t addr, uint16_t dat);
extern uint16_t usDrvOpt3001ReadByte(uint8_t ucRegAddr);

extern uint16_t usDrvOpt3001Sample(void);

#endif // #ifndef __DRV_OPT3001_H