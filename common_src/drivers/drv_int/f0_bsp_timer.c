/**************************************************************************
  Company:
    Self.

  File Name:
    bsp_timer.c

  Description:
    .
  **************************************************************************/

// *****************************************************************************
// Section: File includes
// *****************************************************************************
#include "f0_bsp_timer.h"

// *****************************************************************************
// Section: 变量定义；
// *****************************************************************************

// *****************************************************************************
// Section: 函数定义；
// *****************************************************************************

// @function 初始化并开启定时器；
// @para px_timer_base - 指向要初始化的定时器参数结构体；
// @return 无；
xUserBool_t v_bsp_timer_base_init(xTimerBase_t* px_timer_base)
{
  if(px_timer_base == NULL) {
    LOG_OUT_ERR("定时器参数错误，初始化失败；");

    return USR_FALSE;
  }

  // 开启定时器时钟，以及高级功能配置；
  if (px_timer_base->px_timer_index == TIM1) {
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_TIM1);

    LL_TIM_BDTR_InitTypeDef TIM_BDTRInitStruct;
    // 死区和刹车功能，仅用于高级定时器；
    TIM_BDTRInitStruct.OSSRState = LL_TIM_OSSR_DISABLE;
    TIM_BDTRInitStruct.OSSIState = LL_TIM_OSSI_DISABLE;
    TIM_BDTRInitStruct.LockLevel = LL_TIM_LOCKLEVEL_OFF;
    TIM_BDTRInitStruct.DeadTime = 0;
    TIM_BDTRInitStruct.BreakState = LL_TIM_BREAK_DISABLE;
    TIM_BDTRInitStruct.BreakPolarity = LL_TIM_BREAK_POLARITY_HIGH;
    TIM_BDTRInitStruct.AutomaticOutput = LL_TIM_AUTOMATICOUTPUT_DISABLE;
    // 配置刹车和死区模式，仅用于高级定时器；
    LL_TIM_BDTR_Init(TIM1, &TIM_BDTRInitStruct);
  } else if (px_timer_base->px_timer_index == TIM2) {
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
  }

  LL_TIM_InitTypeDef TIM_TimeBaseStructure;

  // 定时器基本配置；
  TIM_TimeBaseStructure.Prescaler         = TIMER_FIXED_PRESCALER - 1;
  TIM_TimeBaseStructure.CounterMode       = LL_TIM_COUNTERMODE_UP;
  TIM_TimeBaseStructure.RepetitionCounter = 0;
  TIM_TimeBaseStructure.ClockDivision     = LL_TIM_CLOCKDIVISION_DIV1;
  TIM_TimeBaseStructure.Autoreload =
      SystemCoreClock / (px_timer_base->us_freq * TIMER_FIXED_PRESCALER) - 1;
  LL_TIM_Init(px_timer_base->px_timer_index, &TIM_TimeBaseStructure);

  // 如果该定时器工作在非PWM模式下，则开启中断；
  if (px_timer_base->x_is_pwm_mode != USR_TRUE) {
    if (px_timer_base->px_timer_index == TIM1) {
      NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, px_timer_base->x_priority);
      NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
    } else if (px_timer_base->px_timer_index == TIM2) {
      NVIC_SetPriority(TIM2_IRQn, px_timer_base->x_priority);
      NVIC_EnableIRQ(TIM2_IRQn);
    }
  }

  // 启动定时器，并开启输出；
  LL_TIM_EnableCounter(px_timer_base->px_timer_index);
  LL_TIM_EnableAllOutputs(px_timer_base->px_timer_index);

  return USR_TRUE;
}

// @function 初始化PWM通道；默认情况下PWM模式的定时器不开启中断；
//           1、在调用该函数之前，必须先提供定义为：
//            xTimerPwmChannel_t px_pwm_pin_use_table[PWM_CH_NUM]
//           的通道参数配置表，告诉初始化过程中引脚编号、通道编号等信息；
//           该初始化过程会按照参数配置表给定的通道数量和信息自动完成初始化；
//           2、在调用该函数之前，必须先调用
//              ** void v_bsp_timer_base_init(xTimerBase_t* px_timer_base) **
//           对PWM要用到的定时器进行初始化；
// @para 无；
// @return 无；
void v_bsp_timer_pwm_init(void)
{
  LL_GPIO_InitTypeDef     GPIO_InitStructure;
  LL_TIM_OC_InitTypeDef   TIM_OCInitStructure;

  GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStructure.Pull       = LL_GPIO_PULL_DOWN;
  GPIO_InitStructure.Speed      = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStructure.Mode       = LL_GPIO_MODE_ALTERNATE;

  TIM_OCInitStructure.OCMode      = LL_TIM_OCMODE_PWM1;
  TIM_OCInitStructure.OCState     = LL_TIM_OCSTATE_ENABLE;
  TIM_OCInitStructure.OCPolarity  = LL_TIM_OCPOLARITY_HIGH;
  // 下面几个参数仅用于高级定时器；
  TIM_OCInitStructure.OCNState    = LL_TIM_OCSTATE_ENABLE;
  TIM_OCInitStructure.OCNPolarity = LL_TIM_OCPOLARITY_HIGH;
  TIM_OCInitStructure.OCIdleState = LL_TIM_OCIDLESTATE_LOW;
  TIM_OCInitStructure.OCNIdleState = LL_TIM_OCIDLESTATE_LOW;

  for (uint8_t i = 0; i < PWM_CH_NUM; i++) {
    // 开启PWM引脚时钟；
    LL_AHB1_GRP1_EnableClock(px_pwm_pin_use_table[i].ul_pin_clock_source);
    // 配置PWM引脚；
    GPIO_InitStructure.Pin = px_pwm_pin_use_table[i].ul_pin;
    GPIO_InitStructure.Alternate = px_pwm_pin_use_table[i].ul_pin_af;
    LL_GPIO_Init(px_pwm_pin_use_table[i].px_port, &GPIO_InitStructure);

    // 初始化PWM输出通道，并设置默认输出占空比；
    LL_TIM_OC_EnablePreload(px_pwm_pin_use_table[i].px_timer_container,
                            px_pwm_pin_use_table[i].us_pwm_ch);
    TIM_OCInitStructure.CompareValue = (uint32_t)(
        px_pwm_pin_use_table[i].px_timer_container->ARR *
        (px_pwm_pin_use_table[i].us_start_duty * 1.0f / DRV_PWM_MAX_DUTY_STEP));
    LL_TIM_OC_Init(px_pwm_pin_use_table[i].px_timer_container,
                   px_pwm_pin_use_table[i].us_pwm_ch, &TIM_OCInitStructure);
    LL_TIM_OC_DisableFast(px_pwm_pin_use_table[i].px_timer_container,
                          px_pwm_pin_use_table[i].us_pwm_ch);
  }
}

// @function 设置指定PWM通道的占空比；
// @para x_pwm_index - pwm通道编号；
// @para us_duty - 新的占空比级数，最大值：DRV_PWM_MAX_DUTY_STEP；
// @return 无；
void v_bsp_timer_pwm_duty_set(uint8_t uc_pwm_index, uint16_t us_duty)
{
  // 范围限制；
  uint16_t us_final_duty = USER_GET_MIN(us_duty, DRV_PWM_MAX_DUTY_STEP);
  // 计算CCR寄存器的填充值；
  uint32_t ul_ccr_value =
      (uint32_t)(px_pwm_pin_use_table[uc_pwm_index].px_timer_container->ARR *
                 (us_final_duty * 1.0f / DRV_PWM_MAX_DUTY_STEP));
  // 设置CCR寄存器;
  switch (px_pwm_pin_use_table[uc_pwm_index].us_pwm_ch) {
    case LL_TIM_CHANNEL_CH1:
      LL_TIM_OC_SetCompareCH1(
          px_pwm_pin_use_table[uc_pwm_index].px_timer_container, ul_ccr_value);

      break;

    case LL_TIM_CHANNEL_CH2:
      LL_TIM_OC_SetCompareCH2(
          px_pwm_pin_use_table[uc_pwm_index].px_timer_container, ul_ccr_value);

      break;

    case LL_TIM_CHANNEL_CH3:
      LL_TIM_OC_SetCompareCH2(
          px_pwm_pin_use_table[uc_pwm_index].px_timer_container, ul_ccr_value);

      break;

    case LL_TIM_CHANNEL_CH4:
      LL_TIM_OC_SetCompareCH2(
          px_pwm_pin_use_table[uc_pwm_index].px_timer_container, ul_ccr_value);

      break;

    default:
      break;
  }
}
