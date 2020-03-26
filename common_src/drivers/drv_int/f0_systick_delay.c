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


#define delayTICKS_PER_US ((48000000 / SYSTICK_INT_FREQ) / 1000)

void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

// SET SYSCLK = 48M
//void vSystemClockInit(void)
//{
//  LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
//
//  if(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_1) {
//    Error_Handler();
//  }
//  LL_RCC_HSI48_Enable();
//
//  /* Wait till HSI48 is ready */
//  while(LL_RCC_HSI48_IsReady() != 1)
//  {
//
//  }
//  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
//  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
//  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI48);
//
//  /* Wait till System clock is ready */
//  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI48)
//  {
//
//  }
//  // LL_Init1msTick(48000000);
//  LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);
//  LL_SetSystemCoreClock(48000000);
//  LL_RCC_SetUSARTClockSource(LL_RCC_USART2_CLKSOURCE_PCLK1);
//}

// SET SYSCLK = 48M
void vSystemClockInit(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

  if(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_1)
  {
  Error_Handler();
  }
  LL_RCC_HSI_Enable();

   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)
  {

  }
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLL_MUL_6, LL_RCC_PREDIV_DIV_1);
  LL_RCC_PLL_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {

  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

  /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {

  }
  LL_SetSystemCoreClock(48000000);
  LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK1);

  vSystemSystickInit();
}

/* Pointer to Function:
    void systick_initialize(void);

  Description:
    ;

  Parameters:
    None;

  Returns:
    None;
 */
void vSystemSystickInit(void) {
  /* Use frequency provided in argument */
  LL_InitTick(48000000, SYSTICK_INT_FREQ);

  // ����systick�ж�
  NVIC_SetPriority(SysTick_IRQn, 0);
  LL_SYSTICK_EnableIT();
}

void vSystemDelayXus(uint32_t xus) {
  uint32_t ticks = xus * delayTICKS_PER_US;
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
