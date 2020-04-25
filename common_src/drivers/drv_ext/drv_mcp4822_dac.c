
#include "drv_mcp4822_dac.h"

#include "f0_systick_delay.h"

static xMcp4822CsPin_t pul_mcp2822_cs_pins[MCP4822_TOTAL_CH_NUM / 2] = {
    {DRV_MCP4822_CS1_PORT, DRV_MCP4822_CS1_PIN},
    {DRV_MCP4822_CS2_PORT, DRV_MCP4822_CS2_PIN},
    {DRV_MCP4822_CS3_PORT, DRV_MCP4822_CS3_PIN},
    {DRV_MCP4822_CS4_PORT, DRV_MCP4822_CS4_PIN},
    {DRV_MCP4822_CS5_PORT, DRV_MCP4822_CS5_PIN},
    {DRV_MCP4822_CS6_PORT, DRV_MCP4822_CS6_PIN},
};

// @function 初始化MCP4822 io控制脚，并将所有通道设置为最小输出；
// @para 无
// @para 无
void v_mcp4822_init()
{
  {
    LL_GPIO_InitTypeDef GPIO_InitStruct;

    LL_AHB1_GRP1_EnableClock(DRV_MCP4822_PORT_CLK);
    LL_AHB1_GRP1_EnableClock(DRV_MCP4822_CS1_CLK);
    LL_AHB1_GRP1_EnableClock(DRV_MCP4822_CS2_CLK);
    LL_AHB1_GRP1_EnableClock(DRV_MCP4822_CS3_CLK);
    LL_AHB1_GRP1_EnableClock(DRV_MCP4822_CS4_CLK);
    LL_AHB1_GRP1_EnableClock(DRV_MCP4822_CS5_CLK);
    LL_AHB1_GRP1_EnableClock(DRV_MCP4822_CS6_CLK);

    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;

    GPIO_InitStruct.Pin =
        DRV_MCP4822_SDI_PIN | DRV_MCP4822_LDA_PIN | DRV_MCP4822_SCK_PIN;
    LL_GPIO_Init(DRV_MCP4822_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = DRV_MCP4822_CS1_PIN;
    LL_GPIO_Init(DRV_MCP4822_CS1_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = DRV_MCP4822_CS2_PIN;
    LL_GPIO_Init(DRV_MCP4822_CS2_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = DRV_MCP4822_CS3_PIN;
    LL_GPIO_Init(DRV_MCP4822_CS3_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = DRV_MCP4822_CS4_PIN;
    LL_GPIO_Init(DRV_MCP4822_CS4_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = DRV_MCP4822_CS5_PIN;
    LL_GPIO_Init(DRV_MCP4822_CS5_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = DRV_MCP4822_CS6_PIN;
    LL_GPIO_Init(DRV_MCP4822_CS6_PORT, &GPIO_InitStruct);
  }

  // 设置为最小输出
  v_mcp4822_all_set_minimal();
}

// @function 将MCP4822所有驱动通道设置为最小输出；
// @para 无
// @para 无
void v_mcp4822_all_set_minimal(void)
{
  for (uint8_t i = 0; i < MCP4822_TOTAL_CH_NUM; i++) {
    v_mcp4822_ch_value_set(i, 0x0000);
  }
}

// @function 往指定编号的MCP4822写入DA数据；
// @para uc_ch - 将要输出的通道编号；
// @para us_value - DA通道的电压等级；
static void prv_mcp4822_write(uint8_t uc_ch, uint16_t us_value)
{
  if(uc_ch >= MCP4822_TOTAL_CH_NUM) {
    LOG_OUT_ERR("Channel number invalid");

    return;
  }

  // 拉高LDAC引脚电平，不允许将寄存器数据加载到输出通道；
  MCP4822_LDAC_SET();
  // 拉低时钟线，准备开始写操作；
  MCP4822_SCK_CLR();
  // 控制使能引脚；
  // 每颗MCP4822均有两个输出通道，编号为0、1，
  // 如果还有其他MCP48222，则编号自然递增为2、3、4、5...
  v_mcp4822_select(uc_ch / 2);

  // 构造要发送给MCP4822的数据；
  // 其中，
  // bit15表示要写入MCP4822的哪一个输出通道，1表示通道B，0表示通道A；
  // bit13表示输出电压是否在实际基础上放大一倍，1表示不放大，0表示放大；
  // bit12表示输出缓存是否失能，1表示开启，0表示关闭；缓存关闭后将不能输出DA电压；
  uint16_t us_data = (us_value & 0x8fff) | 0x3000;
  // 将数据串行发送给总线；
  for (uint8_t i = 16; i > 0; i--) {
    MCP4822_SCK_CLR();

    if((us_data & 0x8000) == 0x8000) {
      MCP4822_SDI_SET()
    } else {
      MCP4822_SDI_CLR();
    }

    us_data <<= 1;

    vSystemDelayXus(5);
    MCP4822_SCK_SET();
  }

  // deselect mcp4822
  v_mcp4822_unselect(uc_ch / 2);

  // 拉低LDA引脚。允许输出；
  MCP4822_LDAC_CLR();
  vSystemDelayXus(5);
  MCP4822_LDAC_SET();
  MCP4822_SCK_CLR();
}

void v_mcp4822_ch_value_set(uint8_t uc_ch, uint16_t us_value)
{
  if ((uc_ch % 2) == 0) { // 通道A
    prv_mcp4822_write(uc_ch, us_value);
  } else { // 通道B
    prv_mcp4822_write(uc_ch, us_value | 0x8000);
  }
}




