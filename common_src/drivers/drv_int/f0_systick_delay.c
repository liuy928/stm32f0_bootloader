/**************************************************************************
  Company:
    Self.

  File Name:
    bsp_led.c

  Description:
    .
  **************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************
#include "f0_systick_delay.h"

#define TICKS_PER_US ((48000000 / SYSTICK_INT_FREQ) / 1000)

// @function 初始化配置系统时钟；
//            1、使用单片内部时钟；2、PLL倍频到48M；3、开启systick；
// @para 无
// return 无
void v_system_clock_init(void)
{
  // 设置flash延迟时间；
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
  if(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_1) {
    Error_Handler();
  }

  // 开启用于ADC的14M内部时钟；
  LL_RCC_HSI14_Enable();
  while (LL_RCC_HSI14_IsReady() != 1) {
  }
  // 设置内部14M时钟校准；
  LL_RCC_HSI14_SetCalibTrimming(16);

  // 启用内部48M时钟，作为系统主时钟源；
  LL_RCC_HSI48_Enable();
  while (LL_RCC_HSI48_IsReady() != 1) {
  }

  // 设置分频系数，并设置系统时钟；
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI48);
  while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI48) {
  }

  // 设置Systick时钟源；
  LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);
  // 修改系统时钟全局变量；
  LL_SetSystemCoreClock(48000000);

  // 外设时钟设置；
  LL_RCC_HSI14_EnableADCControl(); // ADC外设可开启HSI14时钟；
  LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK1);
  LL_RCC_SetUSARTClockSource(LL_RCC_USART2_CLKSOURCE_PCLK1);

  // 初始化systick中断频率；
  v_system_systick_init();
}

// @function 初始化systick中断频率；并开启中断；
void v_system_systick_init(void)
{
  // 设定systick中断频率；
  LL_InitTick(48000000, SYSTICK_INT_FREQ);

  NVIC_SetPriority(SysTick_IRQn, 0);
  LL_SYSTICK_EnableIT();
}

void vSystemDelayXus(uint32_t xus)
{
  uint32_t ticks = xus * TICKS_PER_US;
  uint32_t told = SysTick->VAL, tnow = 0, tcnt = 0;
  uint32_t reload = SysTick->LOAD;

  while (1) {
    tnow = SysTick->VAL;
    if (tnow != told) {
      if (tnow < told) {
        tcnt += told - tnow;
      } else {
        tcnt += reload - tnow + told;
      }

      told = tnow;
      if (tcnt >= ticks) {
        return;
      }
    }
  }
}

void vSystemDelayXms(uint32_t xms) { LL_mDelay(xms); }

/*******************************************************************************
 End of File
*/
