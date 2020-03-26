#ifndef __BSP_EXTI_H
#define __BSP_EXTI_H

#include "system_includes.h"

#define EXTI_PIN_STATE(port, pin) 		GPIO_ReadInputDataBit(port, pin) 

#define DRV_EXTI_BTN_OFF_SET     0x00
#define DRV_EXTI_BTN_AUTO_SET    0x01
#define DRV_EXTI_BTN_MODE1_SET   0x02
#define DRV_EXTI_BTN_MODE2_SET   0x03
#define DRV_EXTI_BTN_OFF_RESET   0x04

extern uint8_t exti_flag;



extern void bsp_exti_init(SYS_INT_PRIORITY prio);	 //外部中断初始化

#endif
