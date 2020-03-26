#ifndef __DRV_IIC_PCF8563_H
#define __DRV_IIC_PCF8563_H
#include "system_includes.h"

#define PCF8563_DEV_ADDR    0xa2

typedef struct
{
	uint8_t year;   //2000+year
	uint8_t month;
  uint8_t day;
	uint8_t week;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
}SystemRTCTimerStruct;

extern SystemRTCTimerStruct pcf8563_init_time;
extern SystemRTCTimerStruct system_current_time;

extern void drv_pcf8563_init(SystemRTCTimerStruct *rtc_init_time);

extern ErrorStatus drv_pcf8563_set_time(SystemRTCTimerStruct *time);
extern ErrorStatus drv_pcf8563_get_time(SystemRTCTimerStruct *time);

#endif
