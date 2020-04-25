/**************************************************************************
  Company:
    Self.

  File Name:
    bsp_led.c

  Description:
    .
  **************************************************************************/

#ifndef __BSP_LED_H
#define __BSP_LED_H

// *****************************************************************************
// Section: File includes
// *****************************************************************************
#include "system_platform_config.h"

// *****************************************************************************
// Section: Data Types.
// *****************************************************************************
typedef struct {
  GPIO_TypeDef *px_port;
  uint32_t     ul_pin;
  uint32_t     ul_clock_source;
} xLedPins_t;

// *****************************************************************************
// Section: Interface export.
// *****************************************************************************
extern xLedPins_t px_led_pin_use_table[LED_NUM];

extern void v_bsp_led_init(void);

extern void v_bsp_led_toggle(xTableLedIndex_t x_index);
extern void v_bsp_led_set(xTableLedIndex_t x_index, xUserLevel_t x_status);

#endif // #ifndef __BSP_LED_H

/*******************************************************************************
 End of File
*/