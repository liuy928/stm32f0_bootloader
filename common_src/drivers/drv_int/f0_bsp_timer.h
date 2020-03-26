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
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************
#include "system_includes.h"


// *****************************************************************************
// *****************************************************************************
// Section: Data Types.
// *****************************************************************************
// *****************************************************************************
#define TIMER_FIXED_PRESCALER       1

typedef enum
{
  kChannel1  = 0x01,
  kChannel2  = 0x02,
  kChannel3  = 0x04,
  kChannel4  = 0x08,
}TimerOCChannelTable;

typedef enum{
  BSP_TIMER_OPEN_NORMAL           = 0x00,
  BSP_TIMER_OPEN_UPDATE_INT       = 0x01,    
  BSP_TIMER_OPEN_PWM_OUT          = 0x02,             
}BSP_TIMER_OPEN_MODE;

typedef struct
{
    TIM_TypeDef           *ID;
    IRQn_Type             irq;
    uint16_t              freq;  //interrupt freq
    
    SYS_INT_PRIORITY      priority;
    uint8_t               open_mode;
    uint8_t               oc_channel_mask;
}BSP_TIMER_HANLDE;

#if SYSTEM_USING_TIM1
extern BSP_TIMER_HANLDE drvTimer1;
#endif

#if SYSTEM_USING_LED_TIMER_GROUP1
extern BSP_TIMER_HANLDE drv_timer_led_group1;
#endif 

#if SYSTEM_USING_LED_TIMER_GROUP2
extern BSP_TIMER_HANLDE drv_timer_led_group2;
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Interface export.
// *****************************************************************************
// *****************************************************************************
extern void bsp_timer_initialize(BSP_TIMER_HANLDE* timer_index, 
                                 SYS_INT_PRIORITY prio, uint8_t open_mode, uint8_t oc_channel);
extern uint8_t bsp_pwm_led_adjustment(BSP_TIMER_HANLDE* timer_index, uint32_t ch, float pwm_duty);

#endif // #ifndef __BSP_TIMER_H
