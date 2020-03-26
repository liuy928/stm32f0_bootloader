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
typedef enum{ kTurnOn = 1, kTurnOff = 0, }LedStatusTable;
typedef enum{ kLed3 = 3, kLed4 = 4, kLed5 = 5, kLed6 = 6, }LedIndexTable;


// *****************************************************************************
// Section: Interface export.
// *****************************************************************************
extern void vBspLedInit(void);

extern void vBspLedToggle(LedIndexTable led_index);
extern void vBspLedSet(LedIndexTable led_index,
                       LedStatusTable status);

#endif // #ifndef __BSP_LED_H

/*******************************************************************************
 End of File
*/