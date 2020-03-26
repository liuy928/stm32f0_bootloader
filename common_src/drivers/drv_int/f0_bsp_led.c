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
#include "f0_bsp_led.h"

/* Pointer to Function:   
    void bsp_led_initialize(void);

  Description:
    This function supports the initialization of the led pins and clock;
    After initialization the all pins will be set to low level;

  Parameters:
    None;

  Returns:
    None;
 */
void vBspLedInit(void)
{
  LL_GPIO_InitTypeDef  GPIO_InitStruct;

  LL_AHB1_GRP1_EnableClock(DRV_LED3_CLK);
  
  GPIO_InitStruct.Speed         = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Mode          = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.OutputType    = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull          = LL_GPIO_PULL_UP;  
  GPIO_InitStruct.Pin           = DRV_LED3_PIN;
  LL_GPIO_Init(DRV_LED3_PORT, &GPIO_InitStruct);
  
  LL_GPIO_SetOutputPin(DRV_LED3_PORT, DRV_LED3_PIN);
}

/* Pointer to Function:   
    void bsp_led_toggle(LedIndexTable led_index);

  Description:
    This function supports toggle the level of a led pin;

  Parameters:
    - led_index- index of the LED will be toggle;

  Returns:
    None;
 */
void vBspLedToggle(LedIndexTable led_index)
{
  switch(led_index)
  {
  case kLed3:
    LL_GPIO_TogglePin(DRV_LED3_PORT, DRV_LED3_PIN);
  
    break;
  
#ifdef LED_ARRAY
  case kLed4:
    ;
    
    break;
    
  case kLed5:
    ;

    break;
    
  case kLed6:
    ;
    
    break;
#endif
  }
}

/* Pointer to Function:   
    void bsp_led_control(LedIndexTable led_index, LedStatusTable status);

  Description:
    This function supports set the level of led pin;

  Parameters:
    - led_index- index of the LED will be set;
    - status- new status of the pin;

  Returns:
    None;
 */
void vBspLedSet(LedIndexTable led_index, LedStatusTable status)
{
  switch(led_index)
  {
  case kLed3:
    (status == kTurnOn)? LL_GPIO_SetOutputPin(DRV_LED3_PORT, DRV_LED3_PIN) : LL_GPIO_ResetOutputPin(DRV_LED3_PORT, DRV_LED3_PIN);; 
   
    break;
 
#ifdef LED_ARRAY    
  case kLed4:
    ; 
    
    break;
    
  case kLed5:
    ; 

    break;
    
  case kLed6:
    ; 
    
    break;
#endif
  }
}

/*******************************************************************************
 End of File
*/