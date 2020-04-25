/**************************************************************************
  Company:
    Self.

  File Name:
    bsp_timer.h

  Description:
    .
  **************************************************************************/

#ifndef __BSP_TIMER_H
#define __BSP_TIMER_H

// *****************************************************************************
// Section: File includes
// *****************************************************************************
#include "system_platform_config.h"

// *****************************************************************************
// Section: Data Types.
// *****************************************************************************
#define TIMER_FIXED_PRESCALER       2

// PWM占空比可调级数；在修改PWM占空比的时候按照此宏定义作为基数进行传入；
// 例如如果需要50%的占空比，则相关API应该传入:DRV_PWM_MAX_DUTY_STEP / 2；
#define DRV_PWM_MAX_DUTY_STEP       (1000ul)

// 定时器管理结构体；
typedef struct {
  TIM_TypeDef*      px_timer_index;   // 定时器索引；
  uint16_t          us_freq;          // P中断频率，单位Hz；

  xSysIntPriority_t x_priority;       // 定时器中断优先级；
  xUserBool_t       x_is_pwm_mode;    // 是否运行在PWM模式；
} xTimerBase_t;

// PWM通道管理结构体；
typedef struct {
  TIM_TypeDef*  px_timer_container; // PWM通道所属的定时器；
  uint16_t      us_pwm_ch;          // pwm通道号；
  uint16_t      us_start_duty;      // pwm信号起始占空比，最大值：DRV_PWM_MAX_DUTY_STEP；

  // 通道引脚信息；
  GPIO_TypeDef  *px_port;
  uint32_t      ul_pin;
  uint32_t      ul_pin_af;
  uint32_t      ul_pin_clock_source;
} xTimerPwmChannel_t;

#ifndef PWM_CH_NUM
#define PWM_CH_NUM 1
#endif // #ifndef PWM_CH_NUM

// 仅声明，需要在应用层定义并赋值；
extern xTimerBase_t x_bsp_timer1_base;
extern xTimerBase_t x_bsp_timer2_base;
extern xTimerPwmChannel_t px_pwm_pin_use_table[PWM_CH_NUM];

// *****************************************************************************
// Section: Interface export.
// *****************************************************************************
extern xUserBool_t v_bsp_timer_base_init(xTimerBase_t* px_timer_base);
extern void v_bsp_timer_pwm_init(void);
extern void v_bsp_timer_pwm_duty_set(uint8_t uc_pwm_index, uint16_t us_duty);

#endif // #ifndef __BSP_TIMER_H
