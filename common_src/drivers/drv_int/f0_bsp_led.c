/**************************************************************************
  Company:
    Self.

  File Name:
    bsp_led.c

  Description:
    .
  **************************************************************************/

// *****************************************************************************
// Section: File includes
// *****************************************************************************
#include "f0_bsp_led.h"

// @function 初始化LED控制引脚；
//           在调用该函数之前，必须先提供定义为：
//            xLedPins_t px_led_pin_use_table[LED_CH_NUM]
//           的通道参数配置表，告诉初始化过程中引脚编号、通道编号等信息；
//           该初始化过程会按照参数配置表给定的通道数量和信息自动完成初始化；
void v_bsp_led_init(void)
{
  LL_GPIO_InitTypeDef GPIO_InitStruct;

  GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Mode       = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull       = LL_GPIO_PULL_UP;

  for (uint8_t i = 0; i < LED_NUM; i++) {
    LL_AHB1_GRP1_EnableClock(px_led_pin_use_table[i].ul_clock_source);

    GPIO_InitStruct.Pin = px_led_pin_use_table[i].ul_pin;
    LL_GPIO_Init(px_led_pin_use_table[i].px_port, &GPIO_InitStruct);

    // 初始化后关闭LED；
    v_bsp_led_set((xTableLedIndex_t)i, OFF);
  }
}

// @function 控制指定的LED翻转；
void v_bsp_led_toggle(xTableLedIndex_t x_index)
{
  if (x_index >= LED_NUM) {
    return;
  }

  LL_GPIO_TogglePin(px_led_pin_use_table[x_index].px_port,
      px_led_pin_use_table[x_index].ul_pin);
}

// @function 将指定的LED点亮或熄灭；
void v_bsp_led_set(xTableLedIndex_t x_index, xUserLevel_t x_status)
{
  (x_status == ON)
      ? LL_GPIO_ResetOutputPin(px_led_pin_use_table[x_index].px_port,
          px_led_pin_use_table[x_index].ul_pin)
      : LL_GPIO_SetOutputPin(px_led_pin_use_table[x_index].px_port,
          px_led_pin_use_table[x_index].ul_pin);
}

/*******************************************************************************
 End of File
*/