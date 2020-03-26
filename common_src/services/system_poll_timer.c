/**************************************************************************
  Company:
    Self.
    
  File Name:
    bsp_led.c

  Description:
    .
  **************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************
#include "system_poll_timer.h"

// variables

xSystickTimerHandle_t x_systick_timer_handle;

/* Pointer to Function:
    void sys_poll_timer_flag_manage(void);

  Description:
    This function supports set different timeout flag, such as 1ms, 10ms, 20ms,
    50ms, 100ms, 200ms, 500ms, 1000ms;

    If the time is out, the corresponding flag bit will be set, the application
    judge the flag bit was set or not, to deal with something, and if the bit
  was the application must clr the flag bit immediately;

    This function not directly call by application, is should be call period by
    interrupt period 1ms;

  Parameters:
    None;

  Returns:
    None;
 */
void vSystemPollTimeFlagManage(void) {
  x_systick_timer_handle.tick_countor++;

  if (x_systick_timer_handle.tick_countor == 10000)
    x_systick_timer_handle.tick_countor = 0;

  if (x_systick_timer_handle.tick_countor % _0001ms_timeout == 0)
    x_systick_timer_handle._timeout_flag._bit._0001ms_bit = kSysTimeoutSet;

  if (x_systick_timer_handle.tick_countor % _0010ms_timeout == 0)
    x_systick_timer_handle._timeout_flag._bit._0010ms_bit = kSysTimeoutSet;

  if (x_systick_timer_handle.tick_countor % _0020ms_timeout == 0)
    x_systick_timer_handle._timeout_flag._bit._0020ms_bit = kSysTimeoutSet;

  if (x_systick_timer_handle.tick_countor % _0050ms_timeout == 0)
    x_systick_timer_handle._timeout_flag._bit._0050ms_bit = kSysTimeoutSet;

  if (x_systick_timer_handle.tick_countor % _0100ms_timeout == 0)
    x_systick_timer_handle._timeout_flag._bit._0100ms_bit = kSysTimeoutSet;

  if (x_systick_timer_handle.tick_countor % _0200ms_timeout == 0)
    x_systick_timer_handle._timeout_flag._bit._0200ms_bit = kSysTimeoutSet;

  if (x_systick_timer_handle.tick_countor % _0500ms_timeout == 0)
    x_systick_timer_handle._timeout_flag._bit._0500ms_bit = kSysTimeoutSet;

  if (x_systick_timer_handle.tick_countor % _1000ms_timeout == 0)
    x_systick_timer_handle._timeout_flag._bit._1000ms_bit = kSysTimeoutSet;
}

void sys_poll_timer_flag_process() { ; }
