/**************************************************************************
  Company:
    Self.
    
  File Name:
    drv_dac108s085.c

  Description:
    .                                                         
  **************************************************************************/

//提醒：DAC108S085通信时钟不能太低，最好配置在24M左右，器件最高支持30M

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************
#include "drv_dac108s085.h"  

//通道电压级数，范围1~1024，0表示关闭。
uint16_t dac_ch_val_arr[8 * DAC108S_DEV_NUM] = 
{512, 512, 512, 512, 512, //A1,A2,A3,A4,A5(R1,G1,B1,W1,A1)
512, 512, 512, 512, 512,//B1,B2,B3,B4,B5(R2,G2,B2,W2,A2)
512, 512, 512, 512, 512,
512, 512, 512, 512, 512,
512, 512, 512, 512, 512,
512, 512, 512, 512, 512,
512, 512, 512, 512, 512,
512, 512, 512, 512, 512};

void drv_dac108s085_cs_init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure; 
  
  RCC_AHBPeriphClockCmd(DRV_DAC108S_CS_PORT_CLK, ENABLE);
  
  GPIO_InitStructure.GPIO_Speed   =   GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Pin     =   DRV_DAC108S_CS_PIN;        
  GPIO_InitStructure.GPIO_Mode    =   GPIO_Mode_OUT; 	      //推挽输出
  GPIO_InitStructure.GPIO_OType   =   GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd    =   GPIO_PuPd_UP;
  GPIO_Init(DRV_DAC108S_CS_PORT, &GPIO_InitStructure);
  
  //配置为寄存器模式
  drv_dac108s085_mode_set(CMD_MODE_WRM);
  //DAC引脚配置为对地100K下拉
  drv_dac108s085_mode_set(CMD_2K5_OUT | 0xff);
  
  drv_dac108s085_multi_dev_all_ch_output((uint16_t*)dac_ch_val_arr);
}

//写dac108s寄存器，只有一个对外寄存器，通过对不同位的修改，实现不同功能
static void _drv_dac108s085_reg_write(uint16_t cmd)
{
  bsp_spi_exchange_data(DAC108S_SPI, (uint8_t)((cmd >> 8) & 0xff));
  bsp_spi_exchange_data(DAC108S_SPI, (uint8_t)(cmd & 0xff));
}

//设置DAC108S工作在WTM（穿透模式）或 WRM模式（寄存器模式）
void drv_dac108s085_mode_set(uint16_t mode)
{
  uint8_t j = 0;
    
  //select dac108
  dac108s_falsh_cs(0);
  
  //按照菊花链上的器件数量，多次发送，确保每个器件都会收到配置命令
  for(j = 0; j < DAC108S_DEV_NUM; j++)
  {
    _drv_dac108s085_reg_write(mode);
  }
  
  //deselect dac108
  dac108s_falsh_cs(1);  
}

//设置WRM模式下，更新通道寄存器数据到对应的通道脚
static void _drv_dac108s085_wrm_output(void)
{
  uint8_t j = 0;
    
  //select dac108
  dac108s_falsh_cs(0);
  
  //按照菊花链上的器件数量，多次发送，确保每个器件都会收到配置命令
  for(j = 0; j < DAC108S_DEV_NUM; j++)
  {
    _drv_dac108s085_reg_write(CMD_UPDATE_SEL_CH | 0x00ff);//输出
  }
  
  //deselect dac108
  dac108s_falsh_cs(1); 
}

//传入DAC通道值
static void _drv_dac108s085_write(DAC108S_CH_TABLE ch_index, uint16_t dac_val)
{
  uint16_t temp =0;
    
  //阈值检测
  if(dac_val > 1023)
    dac_val = 1023;
    
  //设置寄存器DA值字段
  temp = DAC_CH_MASK + (dac_val << 2);
  //设置通道编号字段
  temp = temp + (ch_index << 12);
  //传入器件
  _drv_dac108s085_reg_write(temp);
}

//菊花链上首个器件，单通道更新，并输出
void drv_dac108s085_single_dev_single_ch_output(DAC108S_CH_TABLE ch_index, uint16_t dac_val)
{
  //enable dac108
  dac108s_falsh_cs(0);
  
  //更新通道寄存器
  _drv_dac108s085_write(ch_index, dac_val);
  
  //disable dac108
  dac108s_falsh_cs(1);
  
    //enable dac108
  dac108s_falsh_cs(0);
  
  //输出
  _drv_dac108s085_reg_write(CMD_UPDATE_SEL_CH | (uint16_t)(1 << ch_index));//输出
  
    //disable dac108
  dac108s_falsh_cs(1);
}

//多个器件所有通道更新，通过修改VAL数组然后下发到器件
void drv_dac108s085_multi_dev_all_ch_output(uint16_t *dac_val)
{
  int8_t j = 0;
  DAC108S_CH_TABLE ch_index = CHA; 
  //通道值暂存，和实际器件数一一对应
  uint16_t ch_val[DAC108S_DEV_NUM];

  //轮询每个通道，同步更新所有器件的相同通道
  for(ch_index = CHA; ch_index < DAC108S_CH_PER_DEV; ch_index++)
  {
    //获取当前要更新的通道数据
    memcpy((uint8_t*)ch_val, (uint8_t*)(dac_val + ch_index * DAC108S_DEV_NUM), 2 * DAC108S_DEV_NUM);
    
    //准备发送数据
    dac108s_falsh_cs(0);
    //轮询所有器件，更新当前通道，注意先写入的数据会更新到菊花链最尾端的器件
    for(j = DAC108S_DEV_NUM - 1; j >= 0; j--)  
    {
      
      _drv_dac108s085_write(ch_index, ch_val[j]);
       
    }
    //准备写入到寄存器
    dac108s_falsh_cs(1);
  }
  
  //所有器件同步输出
  _drv_dac108s085_wrm_output();//输出
}

void drv_dac108s085_test(void)
{
  uint16_t i = 0, k = 0;
  static uint16_t j = 0;
  
  while(1)
  {
  
//      for(i = 1; i < 1025; i++)
//      {
//        for(k = 0; k < 40; k++)
//        {
//          dac_ch_val_arr[k] = i;
//        }
//        drv_dac108s085_multi_dev_all_ch_output(dac_ch_val_arr);
//        
//        systick_delay_xms(5);
//      }
        
   
    
    if(j == 0)
    {
      for(i = 2; i <= 2; i++)
      {
        
        drv_dac108s085_single_dev_single_ch_output(CHA, i - 1);
        drv_dac108s085_single_dev_single_ch_output(CHB, i - 1);
        drv_dac108s085_single_dev_single_ch_output(CHC, i - 1);
        drv_dac108s085_single_dev_single_ch_output(CHD, i - 1);
        drv_dac108s085_single_dev_single_ch_output(CHE, i - 1);
        drv_dac108s085_single_dev_single_ch_output(CHF, i - 1);
        drv_dac108s085_single_dev_single_ch_output(CHG, i - 1);
        drv_dac108s085_single_dev_single_ch_output(CHH, i - 1);
        //_drv_dac108s085_wrm_output();
        systick_delay_xms(1);
      }
      if(i == 1025)
      {
        j = 1;
      }
    }
    else
    {
      for(i = 1024; i >= 1; i--)
      {
        //usb_power_pin(1);
        drv_dac108s085_single_dev_single_ch_output(CHA, i - 1);
        drv_dac108s085_single_dev_single_ch_output(CHB, i - 1);
        drv_dac108s085_single_dev_single_ch_output(CHC, i - 1);
        drv_dac108s085_single_dev_single_ch_output(CHD, i - 1);
        drv_dac108s085_single_dev_single_ch_output(CHE, i - 1);
        
        //_drv_dac108s085_wrm_output();
        systick_delay_xms(1);
      }
      
      if(i == 0)
      {
        //systick_delay_xms(2500);
        
        systick_delay_xms(100);
        j = 0;
      }
      else
      {
        //usb_power_pin(1);
      }
    }
  }
}



