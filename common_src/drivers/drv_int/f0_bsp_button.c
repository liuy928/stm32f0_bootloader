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
#include "f0_bsp_button.h"
#include "sys_fifo.h"
//#include "sys_light_mode.h"


// variables
xFifo_t *xBtnBuffer = NULL;

// local function
static xUserBitState_t prvBspButtonPinRead(BTN_INDEX xIndex);
static uint8_t vBspButtonScan(BTN_INDEX xIndex);


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
void vBspButtonInit(void)
{
  LL_GPIO_InitTypeDef GPIO_InitStruct;

  LL_AHB1_GRP1_EnableClock(DRV_BTN_CLK);

  GPIO_InitStruct.Speed         = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Mode          = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull          = LL_GPIO_PULL_UP;
  GPIO_InitStruct.Pin           = DRV_BTN_MENU_PIN;
  LL_GPIO_Init(DRV_BTN_PORT, &GPIO_InitStruct);

	xBtnBuffer = px_fifo_create(BUTTON_FIFO_SIZE);
}

// 10ms周期调用；
void vBspButtonPush(void)
{
  vBspButtonPutOneBtn(BTN_MENU);
}

uint8_t xBspButtonPop(void)
{
  uint8_t ret = kNone;
  int16_t xBufRes = -1;

  // 读取
  xBufRes = us_fifo_get(xBtnBuffer, (uint8_t*)&ret, 1);
  if(xBufRes != -1) {
    return ret;
  } else {
    return kNone;
  }
}

void vBspButtonPutOneBtn(BTN_INDEX xIndex) {
  static uint8_t btn_state[BTN_NUM] = {BTN_STATE0},
                 btn_count[BTN_NUM] = {0};
  uint8_t btn_value[BTN_NUM] = {kNone};
  uint8_t uc_key_value = kNone;

  // 扫描按键
  btn_value[xIndex] = vBspButtonScan(xIndex);
  // 判断双击
  switch(btn_state[xIndex]) {
    case BTN_STATE0:
      if(btn_value[xIndex] == ((xIndex << 4) + kShort)) {  // 检测到单击
        // btn_state[xIndex] = BTN_STATE1; // TODO: 如果需要使用双击检测，使用该语句；
        uc_key_value = (xIndex << 4) + kShort; // TODO： 如果不用检测双击使用该语句；
        us_fifo_put(xBtnBuffer, &uc_key_value, 1);

        btn_count[xIndex] = 0;
      } else if(btn_value[xIndex] == ((xIndex << 4) + kLong)) { // 长按
        uc_key_value = btn_value[xIndex];
        us_fifo_put(xBtnBuffer, &uc_key_value, 1);
      }

      break;

    case BTN_STATE1:
      if(btn_value[xIndex] == ((xIndex << 4) + kShort)) { // 双击
        uc_key_value = (xIndex << 4) + kDouble;
        us_fifo_put(xBtnBuffer, &uc_key_value, 1);

        btn_state[xIndex] = BTN_STATE0;
      } else {
        if(++btn_count[xIndex] >= 1) { // 10ms双击等待
          uc_key_value = (xIndex << 4) + kShort;
          us_fifo_put(xBtnBuffer, &uc_key_value, 1);

          btn_state[xIndex] = BTN_STATE0;
          btn_count[xIndex] = 0;
        }
      }

      break;
  }
}

static xUserBitState_t prvBspButtonPinRead(BTN_INDEX xIndex)
{
	xUserBitState_t bit_status = USR_BIT_SET;

  switch(xIndex)
  {
  case BTN_MENU:
    bit_status = (xUserBitState_t)(LL_GPIO_IsInputPinSet(DRV_BTN_PORT, DRV_BTN_MENU_PIN));

    break;
  }

  return bit_status;
}

static uint8_t vBspButtonScan(BTN_INDEX xIndex)
{
  static uint8_t btn_state[BTN_NUM] = {BTN_STATE0},
                 btn_count[BTN_NUM] = {0};
  uint8_t btn_value[BTN_NUM] = {kNone};



  switch(btn_state[xIndex]) {
    case BTN_STATE0:
      if(!prvBspButtonPinRead(xIndex)) {  // 检测到按键按下
        btn_state[xIndex] = BTN_STATE1;
      }

      break;

    case BTN_STATE1:
      if(!prvBspButtonPinRead(xIndex)) { // DEBANCE, 去抖动，按键依旧按下
        btn_state[xIndex] = BTN_STATE2;
      } else {
        btn_state[xIndex] = BTN_STATE0;
      }

      break;

    case BTN_STATE2:
      if(prvBspButtonPinRead(xIndex)) { // 按键被释放
        btn_value[xIndex] = (xIndex << 4) + kShort;
        // 复位状态机
        btn_state[xIndex] = BTN_STATE0;
      } else if(btn_count[xIndex]++ >= BUTTON_LONG_PRESS_TIMEOUT) { // 超时，认为是长按
        btn_value[xIndex] = (xIndex << 4) + kLong;
        btn_state[xIndex] = BTN_STATE3;
        btn_count[xIndex] = 0;
      }

      break;

    case BTN_STATE3:
      if(prvBspButtonPinRead(xIndex)) {
        btn_state[xIndex] = BTN_STATE0;
      }

      break;
  }

  return btn_value[xIndex];
}

