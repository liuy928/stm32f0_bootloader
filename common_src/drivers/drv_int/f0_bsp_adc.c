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
#include "f0_bsp_adc.h"
#include <stdlib.h>

// variables
uint16_t regular_adc_data_table[ADC_MOST_SAMPLE_CH_NUM];

uint8_t adc_sample_flag = 0;

BSP_ADC_HANDLE drvDefaultADCSet = {DRV_ADC_INDEX,               //ADC模块
                          ADC1_COMP_IRQn,             //中断向量
                            DMA1_Channel1_IRQn,       //DMA通道中断向量
                              kPrioL3,                //中断优先级
                                0,                    //打开模式
                                  DMA1_Channel1,      //DAM通道
                                    0,                //标志
                                      0               //通道参数
                         };

static void _bsp_adc_pins_config(BSP_ADC_HANDLE *adc)
{
  GPIO_InitTypeDef    GPIO_InitStructure;
  
  /* GPIOA Periph clock enable */
  RCC_AHBPeriphClockCmd(adc->ch->clk, ENABLE);
  
  GPIO_InitStructure.GPIO_Pin     = adc->ch->pin;
  GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
  GPIO_Init(adc->ch->port, &GPIO_InitStructure);
}

static void _bsp_adc_int_config(BSP_ADC_HANDLE *adc)
{
  NVIC_InitTypeDef    NVIC_InitStructure;
  
  NVIC_InitStructure.NVIC_IRQChannel            = adc->irq;
  NVIC_InitStructure.NVIC_IRQChannelPriority    = adc->priority;
  NVIC_InitStructure.NVIC_IRQChannelCmd         = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

static void _bsp_adc_dma_config(BSP_ADC_HANDLE *adc)
{
  DMA_InitTypeDef     DMA_InitStructure;
  NVIC_InitTypeDef    NVIC_InitStructure;
  
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  
  NVIC_InitStructure.NVIC_IRQChannel = adc->irq_dma;     
  NVIC_InitStructure.NVIC_IRQChannelPriority = adc->priority;  
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                      
  NVIC_Init(&NVIC_InitStructure);  
    
  DMA_DeInit(adc->dma_channel);                   
  DMA_StructInit(&DMA_InitStructure); 
  
  DMA_InitStructure.DMA_PeripheralBaseAddr  = (uint32_t)&(ADC1->DR);
  DMA_InitStructure.DMA_MemoryBaseAddr      = (uint32_t)&regular_adc_data_table[0];
  DMA_InitStructure.DMA_DIR                 = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize          = ADC_MOST_SAMPLE_CH_NUM;
  DMA_InitStructure.DMA_PeripheralInc       = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc           = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize      = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode                = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority            = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M                 = DMA_M2M_Disable;
  DMA_Init(adc->dma_channel, &DMA_InitStructure);
  
  DMA_ClearITPendingBit(DMA_IT_TC); 
  DMA_ITConfig(adc->dma_channel, DMA_IT_TC, ENABLE);              
    
  /* DMA1 Channel1 enable */
  DMA_Cmd(adc->dma_channel, ENABLE);
}

static void _bsp_adc_hardware_setup(BSP_ADC_HANDLE *adc)
{
  ADC_InitTypeDef     ADC_InitStructure;
 
  RCC_ADCCLKConfig(RCC_ADCCLK_HSI14);  
  RCC_APB2PeriphClockCmd(DRV_ADC_CLK, ENABLE);

  ADC_DeInit(adc->ID);
  ADC_StructInit(&ADC_InitStructure);

  ADC_InitStructure.ADC_Resolution              = ADC_Resolution_10b;
  ADC_InitStructure.ADC_ContinuousConvMode      = DISABLE; 
  ADC_InitStructure.ADC_ExternalTrigConvEdge    = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_DataAlign               = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_ScanDirection           = ADC_ScanDirection_Backward;
  ADC_Init(adc->ID, &ADC_InitStructure); 
  
  ADC_ITConfig(adc->ID, ADC_IT_EOSEQ, ENABLE);
 
  /* ADC Calibration */
  ADC_GetCalibrationFactor(adc->ID);
  /* Enable the ADC peripheral */
  ADC_Cmd(adc->ID, ENABLE);  
  /* Wait the ADRDY falg */
  while(!ADC_GetFlagStatus(adc->ID, ADC_FLAG_ADRDY)); 
    
  /* Enable ADC_DMA */  
  ADC_DMACmd(adc->ID, ENABLE);
  /* ADC DMA request in circular mode */
  ADC_DMARequestModeConfig(adc->ID, ADC_DMAMode_Circular);
  
  /* ADC1 regular Software Start Conv */ 
  //ADC_StartOfConversion(DRV_ADC_TEMP_INDEX);
}

BSP_ADC_HANDLE bsp_adc_open(BSP_ADC_CH *ch)
{
  
  if(ch == NULL)
  {
    return drvDefaultADCSet;
  }
  BSP_ADC_HANDLE drvADC = drvDefaultADCSet;
  
  drvADC.ch = ch;
  if(drvDefaultADCSet.flag == 0)
  {
    _bsp_adc_int_config(&drvADC);
    _bsp_adc_dma_config(&drvADC);
    _bsp_adc_hardware_setup(&drvADC);
    
    //标记ADC已经初始化
    drvDefaultADCSet.flag = 1;
  }
  _bsp_adc_pins_config(&drvADC);
    /* Convert the ADC1 Channel 1 /  with 13.5 Cycles as sampling time */ 
  ADC_ChannelConfig(drvADC.ID, ch->ch, ADC_SampleTime_13_5Cycles);
  
  return drvADC;
}

//启动ADC转换
void bsp_adc_restart(BSP_ADC_HANDLE *adc)
{
  //allow a new sample sequence
  //if(_isBit_set(ADC_ONCE_SAMPLE_FINISH, adc_sample_flag))
  {
    //_flag_clr_bit(ADC_ONCE_SAMPLE_FINISH, adc_sample_flag);
    ADC_StartOfConversion(adc->ID);
  }
}

//检测是否可以重启转换
void bsp_adc_restart_check(BSP_ADC_HANDLE *adc)
{
  if(!_isBit_set(ADC_ONCE_SAMPLE_FINISH, adc_sample_flag))
  {
    ADC_StartOfConversion(adc->ID);
  }
}

float bsp_adc_get_voltage(BSP_ADC_HANDLE *adc)
{
  uint8_t j = 0;
  uint32_t sum = 0;
  float _voltage = 0.0;
  
  if(!_isBit_set(ADC_READY_RETURN_TEMP, adc_sample_flag))
  {
    return NULL;
  }
  else 
  {
    _flag_clr_bit(ADC_READY_RETURN_TEMP, adc_sample_flag);
  }
  
  for(j = 0; j < ADC_MOST_SAMPLE_ONCE; j++)
  {
    sum += adc->ch->usr_data[j];
  }
  //计算平均值
  sum = sum / ADC_MOST_SAMPLE_ONCE;
  //计算电压
  _voltage = sum / 1024.0 * 3.3;
  
  return _voltage;
}

static float _bsp_voltage_to_temp(float vol_value)
{
  uint8_t i = 0;
  uint32_t vol_integer = 0;
  uint8_t n_node = 4; //小数点后保留位数
  float vol_temp = 0.0; //存储保留小数位后，四舍五入的电压值
  float multi_factor = 1.0;
  float temp_value = TEMP_ERROR;
  
  if(vol_value == NULL)
  {
    return temp_value;
  }
  
  for(i = 0; i < n_node; i++)
    multi_factor *= 10;
  
  vol_integer = (uint32_t)(vol_value * multi_factor);
    
  if(((uint32_t)(vol_value * multi_factor * 10) % 10) >= 5)
    vol_integer += 1;
  
  vol_temp = vol_integer / multi_factor;

  if(vol_temp <= 0.001)
    temp_value = TEMP_ZERO_VOL;
  
  if((vol_temp >= 0.3) && (vol_temp < 0.7))//-20~0
  {
    temp_value = 50 * vol_temp - 35;
  }
  else if((vol_temp >= 0.7) && (vol_temp < 1.5))//0~20
  {
    temp_value = 25 * vol_temp - 17.5;
  }
  else if((vol_temp >= 1.5) && (vol_temp < 2.2))//20~40
  {
    temp_value = 28.5 * vol_temp - 22.75;
  }
  else if((vol_temp >= 2.2) && (vol_temp < 2.7))//40~60
  {
    temp_value = 40 * vol_temp - 48;
  }
  else if((vol_temp >= 2.7) && (vol_temp < 3.0))//60~80
  {
    temp_value = 66.6 * vol_temp - 119.82;
  }
  else if((vol_temp >= 3.0) && (vol_temp < 3.1314))//80~100
  {
    temp_value = 152.2 * vol_temp - 376.6;
  }
  else if((vol_temp >= 3.1314) && (vol_temp < 3.2065))//100~120
  {
    temp_value = 266.3 * vol_temp - 733.9;
  }
  else if(vol_temp >= 3.2065)
  {
    temp_value = 405.8 * vol_temp - 1181.34;
  }
    
  return temp_value;
}

float bsp_adc_get_temperature(float voltage)
{
  float _temperature = TEMP_ERROR;
  
  _temperature = _bsp_voltage_to_temp(voltage);
     
  return _temperature;
}

