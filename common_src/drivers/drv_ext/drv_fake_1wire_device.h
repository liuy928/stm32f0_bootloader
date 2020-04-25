
#ifndef __FAKE_1WIRE_H_
#define __FAKE_1WIRE_H_

// *****************************************************************************
// Section: File includes
// *****************************************************************************
#include "system_platform_config.h"

// *****************************************************************************
// Section: 宏定义
// *****************************************************************************
#define DRV_1WIRE_SET_H()   DRV_1WIRE_PORT->BSRRL = DRV_1WIRE_PIN
#define DRV_1WIRE_SET_L()   DRV_1WIRE_PORT->BSRRH = DRV_1WIRE_PIN

#define DRV_1WIRE_IS_LOW()  ((DRV_1WIRE_PORT->IDR & DRV_1WIRE_PIN) == 0)

#endif // #ifndef __FAKE_1WIRE_H_