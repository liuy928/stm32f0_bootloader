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
#include "bsp_button.h"


// variables

static uint8_t button_fifo[BUTTON_FIFO_SIZE] = {kNone};
BtnValueFIFO btn_fifo = {0, 0, button_fifo};

/* Pointer to Function:   
    void bsp_btn_initialize(void);

  Description:
    This function supports the initialization of the button pins and clock;
    After initialization the all pins will be set to high level;

  Parameters:
    None;

  Returns:
    None;
 */
void bsp_btn_initialize(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(DRV_BTN_CLK, ENABLE);
  
  GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Pin     = DRV_BTN_PIN;
  GPIO_Init(DRV_BTN_PORT, &GPIO_InitStructure);

  //GPIO_SetBits(DRV_BTN_PORT, DRV_BTN_PIN);
}

/* Pointer to Function:   
    static void btn_put(uint8_t val);

  Description:
    This function supports put a button value into the button value FIFO;

  Parameters:
    None;

  Returns:
    None;
 */
static void btn_put(BtnValueTable val)
{
  btn_fifo.fifo[btn_fifo.put_index++] = val;
  
  if(btn_fifo.put_index >= BUTTON_FIFO_SIZE)
    btn_fifo.put_index = 0;
}
  
/* Pointer to Function:   
    static uint8_t button_KeyScan(void)

  Description:
    This function supports scan button to return a button value;

  Parameters:
    None;

  Returns:
    - kShort- short click on button;
    - kLong- long press on button, the press over 1 second;
 */
static BtnValueTable button_KeyScan(void)
{
  static uint8_t btn_state = BTN_STATE0, btn_count = 0;
  BtnValueTable btn_return = kNone;
  
  switch(btn_state)
  {
    case BTN_STATE0:
      if(BTN_STATE)  // detect button press
        btn_state = BTN_STATE1;
      
      break;
      
    case BTN_STATE1:
      if(BTN_STATE)  // DEBANCE, still detect button press
      {
        btn_count = 0;
        btn_state = BTN_STATE2;
      }
      else
        btn_state = BTN_STATE0;
      
      break;
      
    case BTN_STATE2:
      if(!BTN_STATE) // the button have been released
      {
        btn_return = kShort;
        btn_state = BTN_STATE0;
      }
      else if(++btn_count >= 100) // button press more than 1 second, it is a long press
      {
        btn_return = kLong;
        btn_state = BTN_STATE3;
      }
      
      break;
      
    case BTN_STATE3:
      if(!BTN_STATE) // the button have been released
      {
        btn_state = BTN_STATE0;
      }
      else if(++btn_count >= 150) // still detect button press, continuous long press, every 1.5 second return a kLong value
      {
        btn_return = kLong;
        btn_count = 0;
      }
        
      break;
  }
  
  return btn_return;
}

/* Pointer to Function:   
    void bsp_button_put(void)

  Description:
    This function call the function button_KeyScan() to get a button value and 
    put the value into the button value FIFO;
    This function shoulb be call period 10 millisecond;

  Parameters:
    None;

  Returns:
    None;
 */
void bsp_button_put(void)
{
  static uint8_t btn_state = BTN_STATE0, btn_count = 0;
  BtnValueTable btn_temp = kNone;
  
  btn_temp = button_KeyScan();
  
  switch(btn_state)
  {
    case BTN_STATE0:
      if(btn_temp == kShort) // short click?
      {
        btn_count = 0;
        btn_state = BTN_STATE1;
      }
      else // long press, put the value into FIFO
      {
        btn_put(btn_temp);
      }
      
      break;
      
    case BTN_STATE1:
      if(btn_temp == kShort) //another short click? double click
      {
        btn_put(kDouble);
        btn_state = BTN_STATE0;
      }
      else  
      {
        // more than about 500ms still not get another short click, than it is short click.
        if(++btn_count >= 50)  
        {
          btn_put(kShort);
          btn_state = BTN_STATE0;
        }
      }
      
      break;
  }
}

/* Pointer to Function:   
    uint8_t bsp_button_put(void)

  Description:
    This function support applications to get a button value from button value FIFO; 

  Parameters:
    None;

  Returns:
    - BtnValueTable- a button value;
 */
uint8_t bsp_button_get(void)
{
  uint8_t ret = kNone;
    
  if(btn_fifo.put_index == btn_fifo.get_index)
    return ret;
  else
  {
    ret = btn_fifo.fifo[btn_fifo.get_index++];
    btn_fifo.fifo[btn_fifo.get_index - 1] = kNone;
    
    if(btn_fifo.get_index >= BUTTON_FIFO_SIZE)
      btn_fifo.get_index = 0;
    
    return ret;
  }
}
