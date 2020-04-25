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
// Section: File includes
// *****************************************************************************
#include "system_platform_config.h"

// *****************************************************************************
// Section: Data Types.
// *****************************************************************************

typedef struct {
  GPIO_TypeDef *px_port;
  uint32_t     ul_pin;
  uint32_t     ul_clock_source;
  uint32_t     ul_adc_ch;
} xAdcChannel_t;

typedef struct {
  // DMA采集到的数据会放入该数组；
  uint16_t pus_sample_result[ADC_MOST_SAMPLE_ONCE][ADC_CH_NUM];
  // 一次采集结束后会对采集结果求均值，均值会放入该数组；
  uint16_t pus_final_result[ADC_CH_NUM];
} xAdcDataManagement_t;

// *****************************************************************************
// Section: Interface export.
// *****************************************************************************
extern xAdcChannel_t px_adc_pin_use_table[ADC_CH_NUM];

extern xUserBool_t x_bsp_adc_setup(void);

extern void v_bsp_adc_int_callback(void);
extern uint16_t us_bsp_get_adc_value(xTableAdcChannelIndex_t x_ad_ch);
extern float sys_misc_get_temperature(xTableTemperatureIndex_t x_temp_index);

#endif
