#ifndef __BSP_DAC_H
#define __BSP_DAC_H

#include "system_platform_config.h"

// *****************************************************************************
// Section: 宏定义
// *****************************************************************************
#define MCP4822_LDAC_SET()\
    LL_GPIO_SetOutputPin(DRV_MCP4822_PORT, DRV_MCP4822_LDA_PIN);
#define MCP4822_LDAC_CLR()\
    LL_GPIO_ResetOutputPin(DRV_MCP4822_PORT, DRV_MCP4822_LDA_PIN);

#define MCP4822_SCK_SET()\
    LL_GPIO_SetOutputPin(DRV_MCP4822_PORT, DRV_MCP4822_SCK_PIN);
#define MCP4822_SCK_CLR()\
    LL_GPIO_ResetOutputPin(DRV_MCP4822_PORT, DRV_MCP4822_SCK_PIN);

#define MCP4822_SDI_SET()\
    LL_GPIO_SetOutputPin(DRV_MCP4822_PORT, DRV_MCP4822_SDI_PIN);
#define MCP4822_SDI_CLR()\
    LL_GPIO_ResetOutputPin(DRV_MCP4822_PORT, DRV_MCP4822_SDI_PIN)

// *****************************************************************************
// Section: 变量类型定义
// *****************************************************************************
// 用于描述MCP4822的片选引脚；
typedef struct {
  GPIO_TypeDef  *px_port;
  uint32_t      ul_pin;
} xMcp4822CsPin_t;

// *****************************************************************************
// Section: 函数与变量对外声明
// *****************************************************************************
extern xMcp4822CsPin_t pul_mcp2822_cs_pins[MCP4822_TOTAL_CH_NUM / 2];

// @function 选择指定编号的MCP2822；
// @para uc_dev_index - 要选择的器件编号；
// @para 无
#pragma inline
void v_mcp4822_select(uint8_t uc_dev_index)
{
  if(uc_dev_index >= (MCP4822_TOTAL_CH_NUM / 2)) {
    LOG_OUT_ERR("Device index invalid");

    return;
  }

  LL_GPIO_ResetOutputPin(pul_mcp2822_cs_pins[uc_dev_index].px_port,
                         pul_mcp2822_cs_pins[uc_dev_index].ul_pin);
}

// @function 取消选择指定编号的MCP2822；
// @para uc_dev_index - 要取消选择的器件编号；
// @para 无
#pragma inline
void v_mcp4822_unselect(uint8_t uc_dev_index)
{
  if(uc_dev_index >= (MCP4822_TOTAL_CH_NUM / 2)) {
    LOG_OUT_ERR("Device index invalid");

    return;
  }

  LL_GPIO_SetOutputPin(pul_mcp2822_cs_pins[uc_dev_index].px_port,
                       pul_mcp2822_cs_pins[uc_dev_index].ul_pin);
}

extern void v_mcp4822_init();
extern void v_mcp4822_all_set_minimal(void);
extern void v_mcp4822_ch_value_set(uint8_t uc_ch, uint16_t us_value);

#endif