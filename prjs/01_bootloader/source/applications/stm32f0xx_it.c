/**
  ******************************************************************************
  * @file    Project/STM32F0xx_StdPeriph_Templates/stm32f0xx_it.c
  * @author  MCD Application Team
  * @version V1.2.0
  * @date    22-November-2013
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_it.h"

#include "system_poll_timer.h"
#include "ymodem.h"
#include "f0_bsp_usart.h"

/** @addtogroup STM32F0xx_StdPeriph_Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M0 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  vSystemPollTimeFlagManage();
}

/******************************************************************************/
/*                 STM32F0xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f0xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */

#if SYSTEM_USING_USART1
void USART1_IRQHandler(void)
{
  if(LL_USART_IsActiveFlag_ORE(USART1)) {
     LL_USART_ClearFlag_ORE(USART1);
  }

  if(LL_USART_IsActiveFlag_RXNE(USART1)) {
    bsp_usart_isr(&bspUsart1, BSP_USART_EVENT_RXNE);
  }

  if(LL_USART_IsActiveFlag_TXE(USART1)) {
    bsp_usart_isr(&bspUsart1, BSP_USART_EVENT_TXE);
  }

  if(LL_USART_IsActiveFlag_IDLE(USART1)) {
    LL_USART_ClearFlag_IDLE(USART1);
  }
}
#endif /* SYSTEM_USING_USART1 */

/**
  * @brief  This function handles USART2 interrupt request.
  * @param  None
  * @retval None
  */
#if SYSTEM_USING_USART2
void USART2_IRQHandler(void)
{
  if(LL_USART_IsActiveFlag_ORE(USART2)) {
     LL_USART_ClearFlag_ORE(USART2);
  }

  if(LL_USART_IsActiveFlag_RXNE(USART2)) {
    ;
  }

  if(LL_USART_IsActiveFlag_TXE(USART2)) {
    ;
  }

  if(LL_USART_IsActiveFlag_IDLE(USART2)) {
    LL_USART_ClearFlag_IDLE(USART2);

    //调用ymodem包处理函数
    ymodem_int_idle_callback();
  }
}
#endif /* SYSTEM_USING_USART2 */

/**
  * @brief  This function handles USART2 interrupt request.
  * @param  None
  * @retval None
  */
#if SYSTEM_USING_USART3
void USART3_4_IRQHandler(void)
{
  if(LL_USART_IsActiveFlag_ORE(USART3)) {
     LL_USART_ClearFlag_ORE(USART3);
  }

  if(LL_USART_IsActiveFlag_RXNE(USART3)) {
    ;
  }

  if(LL_USART_IsActiveFlag_TXE(USART3)) {
    ;
  }

  if(LL_USART_IsActiveFlag_IDLE(USART3)) {
    LL_USART_ClearFlag_IDLE(USART3);

    //调用ymodem包处理函数
    ymodem_int_idle_callback();
  }
}
#endif /* SYSTEM_USING_USART3 */

#if SYSTEM_USING_USART1
void DMA1_Channel2_3_IRQHandler()
{
  if(DMA_GetITStatus (DMA1_IT_TC3))
  {
    DMA_ClearITPendingBit (DMA1_IT_TC3);
    DMA_ClearITPendingBit (DMA1_IT_GL3);
  }

  if(DMA_GetITStatus(DMA1_IT_TC2))
  {
    DMA_ClearITPendingBit(DMA1_IT_TC2);
    DMA_ClearITPendingBit(DMA1_IT_GL2);
  }
}
#endif

void DMA1_Channel4_5_6_7_IRQHandler()
{
#if SYSTEM_USING_USART2
  //uart2.rx
  if(LL_DMA_IsActiveFlag_HT5(DMA1)) {
    LL_DMA_ClearFlag_HT5(DMA1);
    LL_DMA_ClearFlag_GI5(DMA1);
  } else if(LL_DMA_IsActiveFlag_TC5(DMA1)) {
    LL_DMA_ClearFlag_TC5(DMA1);
    LL_DMA_ClearFlag_GI5(DMA1);
  }

  //uart2.tx
  if(LL_DMA_IsActiveFlag_TC4(DMA1))
  {
     LL_DMA_ClearFlag_TC4(DMA1);
     LL_DMA_ClearFlag_GI4(DMA1);

     if(bspUsart2.open_mode && BSP_USART_OPEN_SIMPLEX) {
       vSystemDelayXus(50);
       vDrvSimplexDirectionSet(&bspUsart2, SIMPLEX_RECV);
     }
  }
#endif

#if SYSTEM_USING_USART3
  //uart3.rx
  if(LL_DMA_IsActiveFlag_HT6(DMA1)) {
    LL_DMA_ClearFlag_HT6(DMA1);
    LL_DMA_ClearFlag_GI6(DMA1);
  } else if(LL_DMA_IsActiveFlag_TC6(DMA1)) {
    LL_DMA_ClearFlag_TC6(DMA1);
    LL_DMA_ClearFlag_GI6(DMA1);
  }

  //uart3.tx
  if(LL_DMA_IsActiveFlag_TC7(DMA1)) {
     LL_DMA_ClearFlag_TC7(DMA1);
     LL_DMA_ClearFlag_GI7(DMA1);

     if(bspUsart3.open_mode && BSP_USART_OPEN_SIMPLEX) {
       vSystemDelayXus(200);
       vDrvSimplexDirectionSet(&bspUsart3, SIMPLEX_RECV);
     }
  }
#endif
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/