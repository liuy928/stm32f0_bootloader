/**************************************************************************
  Company:
    Self.

  File Name:
    bsp_led.c

  Description:
    .
  **************************************************************************/

// *****************************************************************************
// Section: File includes
// *****************************************************************************
#include "f0_bsp_adc.h"

// variables
xAdcDataManagement_t *px_adc_data_management;

// @function 初始化ADC 的DMA采集方式；配置成正常模式；
// @para 无；
// @return 无；
static void prv_bsp_adc_dma_config(void)
{
  LL_DMA_InitTypeDef DMA_InitStructure;

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

  NVIC_SetPriority(DMA1_Channel1_IRQn, PRIO_L3);
  NVIC_EnableIRQ(DMA1_Channel1_IRQn);

  LL_DMA_DeInit(DMA1, LL_DMA_CHANNEL_1);
  LL_DMA_StructInit(&DMA_InitStructure);

  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_1,
                                  LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
  LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PRIORITY_LOW);
  LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MODE_CIRCULAR);

  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1,ADC_MOST_SAMPLE_ONCE * ADC_CH_NUM);
  LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PDATAALIGN_HALFWORD);
  LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_1, (uint32_t) & (ADC1->DR));
  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PERIPH_NOINCREMENT);
  LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MDATAALIGN_HALFWORD);
  LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_1,
                          (uint32_t)&px_adc_data_management->pus_sample_result);
  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MEMORY_INCREMENT);

  /* DMA1 Channel1 enable */
  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
  LL_DMA_ClearFlag_TC1(DMA1);
  LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_1);
}

// @function 初始化ADC模块以及通道引脚；
//           在调用该函数之前，必须先提供定义为：
//            xAdcChannel_t px_adc_pin_use_table[ADC_CH_NUM]
//           的通道参数配置表，告诉初始化过程中引脚编号、通道编号等信息；
//           该初始化过程会按照参数配置表给定的通道数量和信息自动完成初始化；
// @para 无；
// @return xUserBool_t - ADC初始化结果；
xUserBool_t x_bsp_adc_setup(void)
{
  { // IO初始化；
    LL_GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.Mode = LL_GPIO_MODE_ANALOG;
    GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;

    for (uint8_t i = 0; i < ADC_CH_NUM; i++) {
      LL_AHB1_GRP1_EnableClock(px_adc_pin_use_table[i].ul_clock_source);

      GPIO_InitStructure.Pin = px_adc_pin_use_table[i].ul_pin;
      LL_GPIO_Init(px_adc_pin_use_table[i].px_port, &GPIO_InitStructure);
    }
  }

  {  // 创建数据管理空间；
    px_adc_data_management =
        (xAdcDataManagement_t *)calloc(1, sizeof(xAdcDataManagement_t));
    if (px_adc_data_management == NULL) {
      return USR_FALSE;
    }
  }

  // DMA初始化
  prv_bsp_adc_dma_config();

  { // ADC初始化；
    LL_APB1_GRP2_EnableClock(DRV_ADC_CLK);
    LL_ADC_DeInit(ADC1);

    LL_ADC_InitTypeDef ADC_InitStructure;
    LL_ADC_StructInit(&ADC_InitStructure);

    ADC_InitStructure.Clock = LL_ADC_CLOCK_SYNC_PCLK_DIV2;
    ADC_InitStructure.Resolution = LL_ADC_RESOLUTION_10B;
    ADC_InitStructure.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
    ADC_InitStructure.LowPowerMode = LL_ADC_LP_MODE_NONE;
    LL_ADC_Init(ADC1, &ADC_InitStructure);

    LL_ADC_REG_InitTypeDef ADC_REG_InitStruct;

    ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE;
    ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
    ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_CONTINUOUS;
    ADC_REG_InitStruct.DMATransfer = LL_ADC_REG_DMA_TRANSFER_LIMITED;
    ADC_REG_InitStruct.Overrun = LL_ADC_REG_OVR_DATA_PRESERVED;
    LL_ADC_REG_Init(ADC1, &ADC_REG_InitStruct);

    LL_ADC_REG_SetSequencerScanDirection(ADC1, LL_ADC_REG_SEQ_SCAN_DIR_FORWARD);
    LL_ADC_SetSamplingTimeCommonChannels(ADC1, LL_ADC_SAMPLINGTIME_239CYCLES_5);

    // Configure Regular Channel；
    for (uint8_t i = 0; i < ADC_CH_NUM; i++) {
      LL_ADC_REG_SetSequencerChAdd(ADC1, px_adc_pin_use_table[i].ul_adc_ch);
    }

    LL_ADC_DisableIT_EOC(ADC1);
    LL_ADC_DisableIT_EOS(ADC1);

    LL_ADC_Enable(ADC1);
    LL_ADC_REG_StartConversion(ADC1);
  }

  return USR_TRUE;
}

// @function ADC中断回调函数；主要在一次采集结束后，进行均值计算，并开启下一次采集；
// @para 无；
// @return 无；
void v_bsp_adc_int_callback(void)
{
  uint32_t pul_sum[ADC_CH_NUM];
  memset(pul_sum, '\0', ADC_CH_NUM * sizeof(uint32_t));

  for (uint8_t i = 0; i < ADC_MOST_SAMPLE_ONCE; i++) {
    for (uint8_t j = 0; j < ADC_CH_NUM; j++) {
      pul_sum[j] += px_adc_data_management->pus_sample_result[i][j];
    }
  }

  for (uint8_t i = 0; i < ADC_CH_NUM; i++) {
    px_adc_data_management->pus_final_result[i] =
        pul_sum[i] / ADC_MOST_SAMPLE_ONCE;
  }

  // 重启转换；
  LL_ADC_REG_StartConversion(ADC1);
}

// @function 获取指定ADC通道的采集值；
// @para uc_ad_ch - AD通道编号；
// @return 通道采集值，-1表示无效采集；
uint16_t us_bsp_get_adc_value(xTableAdcChannelIndex_t x_ad_ch)
{
  return px_adc_data_management->pus_final_result[x_ad_ch];
}
