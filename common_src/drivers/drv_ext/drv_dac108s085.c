/**************************************************************************
  Company:
    Self.
    
  File Name:
    drv_dac108s085.c

  Description:
    .                                                         
  **************************************************************************/

//���ѣ�DAC108S085ͨ��ʱ�Ӳ���̫�ͣ����������24M���ң��������֧��30M

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************
#include "drv_dac108s085.h"  

//ͨ����ѹ��������Χ1~1024��0��ʾ�رա�
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
  GPIO_InitStructure.GPIO_Mode    =   GPIO_Mode_OUT; 	      //�������
  GPIO_InitStructure.GPIO_OType   =   GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd    =   GPIO_PuPd_UP;
  GPIO_Init(DRV_DAC108S_CS_PORT, &GPIO_InitStructure);
  
  //����Ϊ�Ĵ���ģʽ
  drv_dac108s085_mode_set(CMD_MODE_WRM);
  //DAC��������Ϊ�Ե�100K����
  drv_dac108s085_mode_set(CMD_2K5_OUT | 0xff);
  
  drv_dac108s085_multi_dev_all_ch_output((uint16_t*)dac_ch_val_arr);
}

//дdac108s�Ĵ�����ֻ��һ������Ĵ�����ͨ���Բ�ͬλ���޸ģ�ʵ�ֲ�ͬ����
static void _drv_dac108s085_reg_write(uint16_t cmd)
{
  bsp_spi_exchange_data(DAC108S_SPI, (uint8_t)((cmd >> 8) & 0xff));
  bsp_spi_exchange_data(DAC108S_SPI, (uint8_t)(cmd & 0xff));
}

//����DAC108S������WTM����͸ģʽ���� WRMģʽ���Ĵ���ģʽ��
void drv_dac108s085_mode_set(uint16_t mode)
{
  uint8_t j = 0;
    
  //select dac108
  dac108s_falsh_cs(0);
  
  //���վջ����ϵ�������������η��ͣ�ȷ��ÿ�����������յ���������
  for(j = 0; j < DAC108S_DEV_NUM; j++)
  {
    _drv_dac108s085_reg_write(mode);
  }
  
  //deselect dac108
  dac108s_falsh_cs(1);  
}

//����WRMģʽ�£�����ͨ���Ĵ������ݵ���Ӧ��ͨ����
static void _drv_dac108s085_wrm_output(void)
{
  uint8_t j = 0;
    
  //select dac108
  dac108s_falsh_cs(0);
  
  //���վջ����ϵ�������������η��ͣ�ȷ��ÿ�����������յ���������
  for(j = 0; j < DAC108S_DEV_NUM; j++)
  {
    _drv_dac108s085_reg_write(CMD_UPDATE_SEL_CH | 0x00ff);//���
  }
  
  //deselect dac108
  dac108s_falsh_cs(1); 
}

//����DACͨ��ֵ
static void _drv_dac108s085_write(DAC108S_CH_TABLE ch_index, uint16_t dac_val)
{
  uint16_t temp =0;
    
  //��ֵ���
  if(dac_val > 1023)
    dac_val = 1023;
    
  //���üĴ���DAֵ�ֶ�
  temp = DAC_CH_MASK + (dac_val << 2);
  //����ͨ������ֶ�
  temp = temp + (ch_index << 12);
  //��������
  _drv_dac108s085_reg_write(temp);
}

//�ջ������׸���������ͨ�����£������
void drv_dac108s085_single_dev_single_ch_output(DAC108S_CH_TABLE ch_index, uint16_t dac_val)
{
  //enable dac108
  dac108s_falsh_cs(0);
  
  //����ͨ���Ĵ���
  _drv_dac108s085_write(ch_index, dac_val);
  
  //disable dac108
  dac108s_falsh_cs(1);
  
    //enable dac108
  dac108s_falsh_cs(0);
  
  //���
  _drv_dac108s085_reg_write(CMD_UPDATE_SEL_CH | (uint16_t)(1 << ch_index));//���
  
    //disable dac108
  dac108s_falsh_cs(1);
}

//�����������ͨ�����£�ͨ���޸�VAL����Ȼ���·�������
void drv_dac108s085_multi_dev_all_ch_output(uint16_t *dac_val)
{
  int8_t j = 0;
  DAC108S_CH_TABLE ch_index = CHA; 
  //ͨ��ֵ�ݴ棬��ʵ��������һһ��Ӧ
  uint16_t ch_val[DAC108S_DEV_NUM];

  //��ѯÿ��ͨ����ͬ������������������ͬͨ��
  for(ch_index = CHA; ch_index < DAC108S_CH_PER_DEV; ch_index++)
  {
    //��ȡ��ǰҪ���µ�ͨ������
    memcpy((uint8_t*)ch_val, (uint8_t*)(dac_val + ch_index * DAC108S_DEV_NUM), 2 * DAC108S_DEV_NUM);
    
    //׼����������
    dac108s_falsh_cs(0);
    //��ѯ�������������µ�ǰͨ����ע����д������ݻ���µ��ջ�����β�˵�����
    for(j = DAC108S_DEV_NUM - 1; j >= 0; j--)  
    {
      
      _drv_dac108s085_write(ch_index, ch_val[j]);
       
    }
    //׼��д�뵽�Ĵ���
    dac108s_falsh_cs(1);
  }
  
  //��������ͬ�����
  _drv_dac108s085_wrm_output();//���
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



