#include "f0_bsp_exti.h"

uint8_t exti_flag = 0;

void bsp_exti_init(SYS_INT_PRIORITY prio)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure; 
  GPIO_InitTypeDef GPIO_InitStructure;
  
  RCC_AHBPeriphClockCmd(DRV_EXTI_BTN_PORT_CLK, ENABLE);
  RCC_APB2PeriphClockCmd(DRV_EXTI_CLK, ENABLE);	
  
	GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP; 
	GPIO_InitStructure.GPIO_Pin     = DRV_EXTI_BTN_OFF_PIN;
	GPIO_Init(DRV_EXTI_BTN_PORT, &GPIO_InitStructure);

	EXTI_InitStructure.EXTI_Line    = DRV_EXTI_LINE_BTN_OFF;
	EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt; 
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling; 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
	EXTI_Init(&EXTI_InitStructure);	
    
  SYSCFG_EXTILineConfig (DRV_EXTI_PORT_SRS, DRV_EXTI_BTN_OFF_SRS); 
  
	NVIC_InitStructure.NVIC_IRQChannelPriority = prio; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

  NVIC_InitStructure.NVIC_IRQChannel = EXTI4_15_IRQn;  
	NVIC_Init(&NVIC_InitStructure); 
}
