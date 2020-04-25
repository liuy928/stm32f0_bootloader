/**************************************************************************
  Company:
    Self.

  File Name:
    bsp_led.c

  Description:
    .
  **************************************************************************/

#ifndef __SYSTICK_DELAY_H
#define __SYSTICK_DELAY_H

// *****************************************************************************
// Section: File includes
// *****************************************************************************
#include "system_platform_config.h"

// *****************************************************************************
// Section: Interface export.
// *****************************************************************************
extern void v_system_clock_init(void);
extern void v_system_systick_init(void);
extern void vSystemDelayXus(uint32_t xus);
extern void vSystemDelayXms(uint32_t xms);

#endif // #ifndef __SYSTICK_DELAY_H

/*******************************************************************************
 End of File
*/



























