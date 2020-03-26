/**************************************************************************
  Company:
    Self.
    
  File Name:
    drv_hc595.c

  Description:
    .                                                         
  **************************************************************************/


// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************
#include "drv_hc595.h"  

//一共40个bit，每个bit对应一个通道的开关掩码，字节序从0~4分别对应RGBAW，
//位序从低到高分别对应同一个颜色的ch0~ch7
volatile uint8_t hc595_control_matrix[5] = {0xff, 0xff, 0xff, 0xff, 0xff};

void drv_hc595_initialize(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_AHBPeriphClockCmd(DRV_HC595_CLK, ENABLE);
  
  GPIO_InitStructure.GPIO_Pin     = DRV_HC595_DS | DRV_HC595_SHIFT | DRV_HC595_LOAD;
  GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;  

  GPIO_Init(DRV_HC595_PORT, &GPIO_InitStructure);
  
  drv_hc595_update((uint8_t*)hc595_control_matrix);
}

void drv_hc595_update(uint8_t *ptr)
{
  uint8_t i = 0, j = 0, temp = 0;
  
  for(i = 0; i < HC595_DEV_NUM; i++)
  {
    temp = ptr[i];
    for(j = 0; j < 8; j++)
    {
      //加载数据线
      if(temp & 0x80)
      {
        _drv_hc595_ds_out(1);
      }
      else
      {
        _drv_hc595_ds_out(0);
      }
      
      //写入寄存器
      _bsp_hc595_shift_out(0);
      systick_delay_xus(5);
      _bsp_hc595_shift_out(1);
      systick_delay_xus(5);
      
      temp = temp << 1;
    }
  }
  
  //更新到管脚并锁定
  _bsp_hc595_load_out(0);
  systick_delay_xus(5);
  _bsp_hc595_load_out(1);
  systick_delay_xus(5);
  
  //释放控制线
  _drv_hc595_ds_out(0);
  _bsp_hc595_shift_out(0);
  _bsp_hc595_load_out(0);
}

//输入参数与实际字节的位序为逆序关系，bitindex实际对应的hc595串联后的输出引脚序号；
//HC595串行传输数据的时候高位先行，所以第一个字节的最高位会移位到最后一个器件的Q8，
//最后一个字节的最低位对应的是第一个器件的Q0；
//status表示当前位更新后的状态。
void drv_hc595_bit_control(uint8_t bit_index, FlagStatus status)
{
  uint8_t dev_index = 0, dev_offset = 0, temp = 0;
  
  dev_index = 1 + bit_index / 8;
  dev_offset = bit_index % 8;
  
  temp = hc595_control_matrix[HC595_DEV_NUM - dev_index];
  
  if(status == DISABLE)
  {
    hc595_control_matrix[HC595_DEV_NUM - dev_index] = temp | (1 << dev_offset);
  }
  else if(status == ENABLE)
  {
    hc595_control_matrix[HC595_DEV_NUM - dev_index] = temp & (~(1 << dev_offset));
  }
}