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

xPollTimerHandle_t x_poll_timer;


/* Pointer to Function:
    void sys_poll_timer_flag_manage(void);

  Description:
    This function supports set different timeout flag, such as 1ms, 10ms, 20ms,
    50ms, 100ms, 200ms, 500ms, 1000ms;

    If the time is out, the corresponding flag bit will be set, the application
    judge the flag bit was set or not, to deal with something, and if the bit was
    the application must clr the flag bit immediately;

    This function not directly call by application, is should be call period by
    interrupt period 1ms;

  Parameters:
    None;

  Returns:
    None;
 */
void v_system_poll_time_flag_manage(void)
{
  x_poll_timer.ul_counter++;
  x_poll_timer.ul_counter %= 30000;

  if(x_poll_timer.ul_counter % _0001ms == 0) {
    x_poll_timer.flag.bit._0001ms =  TIMEOUT_SET;
  }

  if(x_poll_timer.ul_counter % _0010ms == 0) {
    x_poll_timer.flag.bit._0010ms =  TIMEOUT_SET;
  }

  if(x_poll_timer.ul_counter % _0020ms == 0) {
    x_poll_timer.flag.bit._0020ms =  TIMEOUT_SET;
  }

  if(x_poll_timer.ul_counter % _0050ms == 0) {
    x_poll_timer.flag.bit._0050ms =  TIMEOUT_SET;
  }

  if(x_poll_timer.ul_counter % _0100ms == 0) {
    x_poll_timer.flag.bit._0100ms =  TIMEOUT_SET;
  }

  if(x_poll_timer.ul_counter % _0200ms == 0) {
    x_poll_timer.flag.bit._0200ms =  TIMEOUT_SET;
  }

  if(x_poll_timer.ul_counter % _0500ms == 0) {
    x_poll_timer.flag.bit._0500ms =  TIMEOUT_SET;
  }

  if(x_poll_timer.ul_counter % _1000ms == 0) {
    x_poll_timer.flag.bit._1000ms =  TIMEOUT_SET;
  }
}
