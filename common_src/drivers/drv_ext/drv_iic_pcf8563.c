#include "drv_iic_pcf8563.h"

#define PCF8563_REG_CTRL1     0x00
#define PCF8563_REG_CTRL2     0x01
#define PCF8563_REG_SEC       0x02
#define PCF8563_REG_MIN       0x03
#define PCF8563_REG_HOUR      0x04
#define PCF8563_REG_DAY       0x05
#define PCF8563_REG_WEEK      0x06
#define PCF8563_REG_MON       0x07
#define PCF8563_REG_YEAR      0x08

SystemRTCTimerStruct pcf8563_init_time = {0x17, 0x01, 0x06, 0x01, 0x00, 0x00, 0x00};
SystemRTCTimerStruct system_current_time;

static uint8_t _drv_pcf8563_write_byte(uint8_t addr, uint8_t dat)
{
  //step1,send out start singnal,for aend address
	bsp_iic2_start();
  
  //step2,send out device address,and set oparate direction
	bsp_iic2_sendbyte(PCF8563_DEV_ADDR | IIC_WR); 
  
  //wait ack
  if(bsp_iic2_wait_ack() != 0)
    goto cmd_error; 
  
  //step3,send out address
	bsp_iic2_sendbyte(addr); 
  
  //wait ack
  if(bsp_iic2_wait_ack() != 0)
    goto cmd_error; 
  
  //step4,send out data
	bsp_iic2_sendbyte(dat);
  
  //wait ack
  if(bsp_iic2_wait_ack() != 0)
    goto cmd_error; 
  
  //step6,transfer complete,send stop singnal
  bsp_iic2_stop();
  
  return 1;
  
cmd_error:
  bsp_iic2_stop();
  return 0;
}
  
static uint8_t _drv_pcf8563_read_byte(uint8_t addr)
{
  uint8_t res;
  
  //step1,send out start singnal,for send address and data
	bsp_iic2_start();
  
  //step2,send out device address,and set oparate direction
	bsp_iic2_sendbyte(PCF8563_DEV_ADDR | IIC_WR); 
  
  //wait ack
  if(bsp_iic2_wait_ack() != 0)
    goto cmd_error; 
  
  //step3,send out address
	bsp_iic2_sendbyte(addr); 
  
  //wait ack
  if(bsp_iic2_wait_ack() != 0)
    goto cmd_error; 
  
  //step4,send out start singnal,for send data
	bsp_iic2_start();
  
  //step5,send out device address,and set oparate direction
	bsp_iic2_sendbyte(PCF8563_DEV_ADDR | IIC_RD); 
  
  //wait ack
  if(bsp_iic2_wait_ack() != 0)
    goto cmd_error; 
    
  res = bsp_iic2_readbyte(IIC_NACK);
  
  //step6,transfer complete,send stop singnal
  bsp_iic2_stop();
  
  return res;
  
cmd_error:
  bsp_iic2_stop();
  return 0xff;
}

void drv_pcf8563_init(SystemRTCTimerStruct *rtc_init_time)
{
  uint8_t pcf8563_flag = 0, flag = 0x5f;

	bsp_iic2_port_config();
  
  drv_spi_flash_read((uint8_t*)(&pcf8563_flag), SPI_FLASH_BASE_ADDR, 1);
  if(pcf8563_flag != 0x5f)
  {
    drv_pcf8563_set_time(rtc_init_time);
    
    //写入初始化标志
    drv_spi_flash_write_check((&flag), SPI_FLASH_BASE_ADDR, 1);
  }

  //启动RTC
  _drv_pcf8563_write_byte(PCF8563_REG_CTRL1, 0x00); 
}

ErrorStatus drv_pcf8563_set_time(SystemRTCTimerStruct *time)
{
  SystemRTCTimerStruct *_time;
  if(time != NULL)
     _time = time;
  
  //判断传入的时间参数是否均合法
  if((_time->year >= 0x99) || 
     ((_time->month == 0) || (_time->month > 0x12)) ||
     ((_time->day == 0) || (_time->day > 0x31)) ||
     (_time->week > 0x07) || (_time->hour >= 0x23) ||
     (_time->min >= 0x59) || (_time->sec >= 0x59))
    return ERROR;
        
  _drv_pcf8563_write_byte(PCF8563_REG_SEC, _time->sec);
  _drv_pcf8563_write_byte(PCF8563_REG_MIN, _time->min);
  _drv_pcf8563_write_byte(PCF8563_REG_HOUR, _time->hour);
  _drv_pcf8563_write_byte(PCF8563_REG_DAY, _time->day);
  _drv_pcf8563_write_byte(PCF8563_REG_WEEK, _time->week);
  _drv_pcf8563_write_byte(PCF8563_REG_MON, _time->month);
  _drv_pcf8563_write_byte(PCF8563_REG_YEAR, _time->year);
  
  return SUCCESS;
}

ErrorStatus drv_pcf8563_get_time(SystemRTCTimerStruct *time)
{
  SystemRTCTimerStruct *_time = time;
    
  _time->sec = _drv_pcf8563_read_byte(PCF8563_REG_SEC) & 0x7f;
  _time->min = _drv_pcf8563_read_byte(PCF8563_REG_MIN) & 0x7f;
  _time->hour = _drv_pcf8563_read_byte(PCF8563_REG_HOUR) & 0x3f;
  _time->day = _drv_pcf8563_read_byte(PCF8563_REG_DAY) & 0x3f;
  _time->week = _drv_pcf8563_read_byte(PCF8563_REG_WEEK) & 0x07;
  _time->month = _drv_pcf8563_read_byte(PCF8563_REG_MON) & 0x1f;
  _time->year = _drv_pcf8563_read_byte(PCF8563_REG_YEAR) & 0xff;
    
  if((_time->sec == 0xff) || (_time->min == 0xff) || (_time->sec == 0xff) ||
     (_time->day == 0xff) || (_time->week == 0xff) || (_time->month == 0xff) || (_time->year == 0xff))
    return ERROR;
  else
    return SUCCESS;  
}

