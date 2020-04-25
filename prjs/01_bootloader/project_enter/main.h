/**************************************************************************
  Company:
    Self.

  File Name:


  Description:
    .
  **************************************************************************/

#ifndef __SYS_CONFIG_H
#define __SYS_CONFIG_H

#include "stm32f0xx_hal.h"

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_utils.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_usart.h"
#include "stm32f0xx_ll_exti.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_usart.h"
#include "stm32f0xx_ll_dma.h"
#include "stm32f0xx_ll_adc.h"
#include "stm32f0xx_ll_tim.h"

// *****************************************************************************
// Section: Interface export.
// *****************************************************************************
extern void _Error_Handler(char *, int);
#define Error_Handler() _Error_Handler(__FILE__, __LINE__)

#endif // #ifndef __SYS_CONFIG_H
/*******************************************************************************
 End of File
*/
