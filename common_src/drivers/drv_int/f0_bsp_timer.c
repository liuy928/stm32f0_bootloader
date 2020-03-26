/**************************************************************************
  Company:
    Self.
    
  File Name:
    bsp_timer.c

  Description:
    .                                                         
  **************************************************************************/


// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************
#include "f0_bsp_timer.h" 

static void _bsp_timer_pins_setup(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  static uint8_t tim_gpio_init_flag = 0;

  if(tim_gpio_init_flag == 1)
    return;

  /* GPIOA, GPIOB Clocks enable */
  RCC_AHBPeriphClockCmd( DRV_LED_GROUP1_CLK | DRV_LED_GROUP2_CLK, ENABLE);

  GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_Level_2;
  GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP ;
  
  GPIO_InitStructure.GPIO_Pin     =  DRV_R_LED_PIN | DRV_G_LED_PIN | DRV_B_LED_PIN | DRV_CW_LED_PIN;
  GPIO_Init(DRV_LED_GROUP1_PORT, &GPIO_InitStructure);
  
  GPIO_PinAFConfig(DRV_LED_GROUP1_PORT, DRV_R_LED_PIN_SRS, GPIO_AF_0);
  GPIO_PinAFConfig(DRV_LED_GROUP1_PORT, DRV_G_LED_PIN_SRS, GPIO_AF_0);
  GPIO_PinAFConfig(DRV_LED_GROUP1_PORT, DRV_B_LED_PIN_SRS, GPIO_AF_0);
  GPIO_PinAFConfig(DRV_LED_GROUP1_PORT, DRV_CW_LED_PIN_SRS, GPIO_AF_0);
  
  GPIO_InitStructure.GPIO_Pin     =  DRV_WW_LED_PIN;
  GPIO_Init(DRV_LED_GROUP2_PORT, &GPIO_InitStructure);  
  
  GPIO_PinAFConfig(DRV_LED_GROUP2_PORT, DRV_WW_LED_PIN_SRS, GPIO_AF_1);
  
  GPIO_ResetBits(DRV_LED_GROUP1_PORT, DRV_R_LED_PIN);
  GPIO_ResetBits(DRV_LED_GROUP1_PORT, DRV_G_LED_PIN);
  GPIO_ResetBits(DRV_LED_GROUP1_PORT, DRV_B_LED_PIN);
  GPIO_ResetBits(DRV_LED_GROUP1_PORT, DRV_CW_LED_PIN);
  GPIO_ResetBits(DRV_LED_GROUP2_PORT, DRV_WW_LED_PIN);
  
  tim_gpio_init_flag = 1;
}

static void _bsp_timer_base_config(BSP_TIMER_HANLDE* timer_index)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  
  /* TIMx clock enable */
  if(timer_index->ID == TIM1)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
  else if(timer_index->ID == TIM3)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
  else if(timer_index->ID == TIM15)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM15, ENABLE);
    
  /* Time Base configuration */
  TIM_TimeBaseStructure.TIM_Prescaler         = TIMER_FIXED_PRESCALER - 1;
  TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period            = SystemCoreClock / (timer_index->freq * TIMER_FIXED_PRESCALER) - 1;
  TIM_TimeBaseStructure.TIM_ClockDivision     = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(timer_index->ID, &TIM_TimeBaseStructure);
}

static void _bsp_timer_int_config(BSP_TIMER_HANLDE* timer_index)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  NVIC_InitStructure.NVIC_IRQChannel            = timer_index->irq; 			        
	NVIC_InitStructure.NVIC_IRQChannelPriority    = timer_index->priority & 0x0f;                              
	NVIC_InitStructure.NVIC_IRQChannelCmd         = ENABLE; 			                                    
	NVIC_Init(&NVIC_InitStructure);

	TIM_ITConfig(timer_index->ID, TIM_IT_Update, ENABLE); 
}

static void _bsp_timer_pwm_config(BSP_TIMER_HANLDE* timer_index)
{
  TIM_OCInitTypeDef  TIM_OCInitStructure;

  /* Channel 1, 2,3 and 4 Configuration in PWM mode */
  TIM_OCInitStructure.TIM_OCMode        = TIM_OCMode_PWM2;
  TIM_OCInitStructure.TIM_OutputState   = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_OCPolarity    = TIM_OCPolarity_Low;
  
  if(timer_index->ID == TIM1)
  {
    TIM_OCInitStructure.TIM_OutputNState  = TIM_OutputNState_Enable;
    TIM_OCInitStructure.TIM_OCNPolarity   = TIM_OCNPolarity_High;
    TIM_OCInitStructure.TIM_OCIdleState   = TIM_OCIdleState_Set;
    TIM_OCInitStructure.TIM_OCNIdleState  = TIM_OCIdleState_Reset;
  }
  
  if(timer_index->oc_channel_mask & kChannel1)
  {
    TIM_OCInitStructure.TIM_Pulse   = 0;//SystemCoreClock / (TIMER_FIXED_PRESCALER * timer_index->freq * 4); //defaule 25% PWM
    TIM_OC1Init(timer_index->ID, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(timer_index->ID, TIM_OCPreload_Enable); 	
  }
  
  if(timer_index->oc_channel_mask & kChannel2)
  {
    TIM_OCInitStructure.TIM_Pulse   = 0;//SystemCoreClock / (TIMER_FIXED_PRESCALER * timer_index->freq * 4); //defaule 25% PWM;
    TIM_OC2Init(timer_index->ID, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(timer_index->ID, TIM_OCPreload_Enable); 	
  }
  
  if(timer_index->oc_channel_mask & kChannel3)
  {
    TIM_OCInitStructure.TIM_Pulse   = 0;//SystemCoreClock / (TIMER_FIXED_PRESCALER * timer_index->freq * 4); //defaule 25% PWM;
    TIM_OC3Init(timer_index->ID, &TIM_OCInitStructure); 
    TIM_OC3PreloadConfig(timer_index->ID, TIM_OCPreload_Enable); 	
  }

  if(timer_index->oc_channel_mask & kChannel4)
  {
    TIM_OCInitStructure.TIM_Pulse   = 0;//SystemCoreClock / (TIMER_FIXED_PRESCALER * timer_index->freq * 4); //defaule 25% PWM;
    TIM_OC4Init(timer_index->ID, &TIM_OCInitStructure); 
    TIM_OC4PreloadConfig(timer_index->ID, TIM_OCPreload_Enable); 	
  }  

  TIM_CtrlPWMOutputs(timer_index->ID, ENABLE);
  TIM_ARRPreloadConfig(timer_index->ID, ENABLE);
}

void bsp_timer_initialize(BSP_TIMER_HANLDE* timer_index, SYS_INT_PRIORITY prio, uint8_t open_mode, uint8_t oc_channel)
{  
  if(timer_index == NULL)
  {
    ;//process error.
    return;
  }
  
  _bsp_timer_pins_setup();
  
  _bsp_timer_base_config(timer_index);
  
  if(open_mode & BSP_TIMER_OPEN_UPDATE_INT)
  {
    timer_index->priority = prio;
    _bsp_timer_int_config(timer_index);
  }
  
  if(open_mode & BSP_TIMER_OPEN_PWM_OUT)
  {
    if(oc_channel & 0x0f == 0)
    {
      oc_channel = kChannel1 | kChannel2 | kChannel3 | kChannel4;
    }
    timer_index->oc_channel_mask = oc_channel;
    _bsp_timer_pwm_config(timer_index);
  }
  
  TIM_Cmd(timer_index->ID, ENABLE);
}

uint8_t bsp_pwm_led_adjustment(BSP_TIMER_HANLDE* timer_index, uint32_t ch, float pwm_duty)
{
  float pwm_value = 0.0;
  
  uint8_t pwm_value_integer = 0, pwm_value_decimal = 0;
  uint16_t pwm_percent = 0;
  
  pwm_value_integer = (uint8_t)pwm_duty;
  pwm_value_decimal = (uint8_t)((pwm_duty - pwm_value_integer) * 100);
  
  if(!((pwm_value_integer < 100) && (pwm_value_decimal <= 99) ||
       ((pwm_value_integer == 100) && (pwm_value_decimal == 0))))
    return 0;
  
  pwm_value = pwm_value_integer / (float)100.0 + pwm_value_decimal / (float)10000.0;
  pwm_percent = (uint16_t)(pwm_value * timer_index->ID->ARR);
  
  if((timer_index->oc_channel_mask & (ch - (uint32_t)timer_index->ID)) == kChannel1)
  {
    TIM_SetCompare1(timer_index->ID, pwm_percent);
  }
  
  if((timer_index->oc_channel_mask & (ch - (uint32_t)timer_index->ID)) == kChannel2)
  {
    TIM_SetCompare2(timer_index->ID, pwm_percent);
  }
  
  if((timer_index->oc_channel_mask & (ch - (uint32_t)timer_index->ID)) == kChannel3)
  {
    TIM_SetCompare3(timer_index->ID, pwm_percent);
  }

  if((timer_index->oc_channel_mask & (ch - (uint32_t)timer_index->ID)) == kChannel4)
  {
    TIM_SetCompare4(timer_index->ID, pwm_percent);
  }
  
  return 1;
}