#ifndef __BSP_IIC_H
#define __BSP_IIC_H

#include "system_platform_config.h"

#define IIC_SDA_OUT_SET(a) 		  if(a)\
                                LL_GPIO_SetOutputPin(DRV_IIC_PORT,DRV_IIC_SDA_PIN);\
                                else\
                                LL_GPIO_ResetOutputPin(DRV_IIC_PORT,DRV_IIC_SDA_PIN)

#define IIC_SCL_OUT_SET(a) 		  if(a)\
                                LL_GPIO_SetOutputPin(DRV_IIC_PORT,DRV_IIC_SCL_PIN);\
                                else\
                                LL_GPIO_ResetOutputPin(DRV_IIC_PORT,DRV_IIC_SCL_PIN)

#define IIC_SDA_IS_SET() 	LL_GPIO_IsInputPinSet(DRV_IIC_PORT, DRV_IIC_SDA_PIN) 
                             
#define IIC_WR      0x00
#define IIC_RD      0x01   
  
#define IIC_NACK    0x00
#define IIC_ACK     0x01
                              
extern void vBspIicInit(void);

extern void vBspIicSendStop(void);
extern void vBspIicSendStart(void);

extern uint8_t vBspIicReadByte(uint8_t iic_ack_flag);
extern void vBspIicWriteByte(uint8_t dat);

extern uint8_t vBspIicWaitAck(void);
extern uint8_t vBspIicCheckDevice(uint8_t _addr);

#endif
