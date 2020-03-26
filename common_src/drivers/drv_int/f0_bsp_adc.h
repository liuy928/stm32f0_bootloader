/**************************************************************************
  Company:
    Self.
    
  File Name:
    bsp_adc.h

  Description:
    .                                                         
  **************************************************************************/

#ifndef __BSP_ADC_H
#define __BSP_ADC_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************
#include "system_includes.h"


// *****************************************************************************
// *****************************************************************************
// Section: Data Types.
// *****************************************************************************
// *****************************************************************************
#define ADC_ONCE_SAMPLE_FINISH   0x00
#define ADC_READY_RETURN_TEMP    0x01

#define TEMP_ERROR              -200.0
#define TEMP_ZERO_VOL           -100.0

typedef struct{
  uint16_t      pin;
  GPIO_TypeDef  *port;
  uint32_t      clk;
  
  uint32_t      ch;
  uint16_t       *usr_data;
}BSP_ADC_CH;

typedef struct 
{
  //for module
  ADC_TypeDef           *ID;
  IRQn_Type             irq;
  IRQn_Type             irq_dma;
   
  SYS_INT_PRIORITY      priority;
  uint8_t               open_mode;
    
  DMA_Channel_TypeDef*  dma_channel;  
  
  uint8_t flag;
  //for channel
  BSP_ADC_CH          *ch;
}BSP_ADC_HANDLE;

// *****************************************************************************
// *****************************************************************************
// Section: Interface export.
// *****************************************************************************
// *****************************************************************************
extern BSP_ADC_HANDLE drvDefaultADCSet;
extern BSP_ADC_HANDLE bspADCLed, bspADCSys, bspADCSwitch;

extern uint16_t regular_adc_data_table[ADC_MOST_SAMPLE_CH_NUM];

extern uint16_t adc_led_sample_table[ADC_MOST_SAMPLE_ONCE];
extern uint16_t adc_sys_sample_table[ADC_MOST_SAMPLE_ONCE];

extern uint8_t adc_sample_flag;

extern BSP_ADC_HANDLE bsp_adc_open(BSP_ADC_CH *ch);

extern void bsp_adc_restart(BSP_ADC_HANDLE *adc);
extern void bsp_adc_restart_check(BSP_ADC_HANDLE *adc);

extern float bsp_adc_get_voltage(BSP_ADC_HANDLE *adc);
float bsp_adc_get_temperature(float voltage);

#endif
