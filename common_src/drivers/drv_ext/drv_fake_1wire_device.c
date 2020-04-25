
#include "drv_fake_1wire_device.h"

// @function 配置1-wire管脚；
// @para 无；
// @return 无；
void v_bsp_1wire_init(void)
{
  LL_GPIO_InitTypeDef GPIO_InitStruct;
  LL_EXTI_InitTypeDef EXTI_InitStruct;

  LL_AHB1_GRP1_EnableClock(DRV_1WIRE_PORT_CLK);

  GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Mode       = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull       = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Pin        = DRV_1WIRE_PIN;
  LL_GPIO_Init(DRV_1WIRE_PORT, &GPIO_InitStruct);

  EXTI_InitStruct.Line_0_31   = DRV_1WIRE_EXTI_LINE;
  EXTI_InitStruct.LineCommand = ENABLE;
  EXTI_InitStruct.Mode        = LL_EXTI_MODE_IT;
  EXTI_InitStruct.Trigger     = LL_EXTI_TRIGGER_FALLING;
  LL_EXTI_Init(&EXTI_InitStruct);

  LL_SYSCFG_SetEXTISource(DRV_1WIRE_EXTI_PORT, LL_SYSCFG_EXTI_LINE11);

  NVIC_SetPriority(EXTI4_15_IRQn, 0);
  NVIC_EnableIRQ(EXTI4_15_IRQn);
}