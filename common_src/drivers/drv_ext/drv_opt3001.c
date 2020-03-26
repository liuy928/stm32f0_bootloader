
#include "sys_command_send.h"

#include "drv_opt3001.h"
#include "f0_bsp_iic.h"

uint8_t ucDrvOpt3001WriteByte(uint8_t addr, uint16_t dat)
{
  //step1,send out start singnal,for aend address
	vBspIicSendStart();
  
  //step2,send out device address,and set oparate direction
	vBspIicWriteByte((OPT3001_I2C_ADDRESS << 1) | IIC_WR); 
  
  //wait ack
  if(vBspIicWaitAck() != 0)
  {
    goto cmd_error;
  }
  
  //step3,send out address
	vBspIicWriteByte(addr); 
  
  //wait ack
  if(vBspIicWaitAck() != 0)
  {
    goto cmd_error; 
  }
  
  //step4,send out data
	vBspIicWriteByte(dat >> 8);
  
  //wait ack
  if(vBspIicWaitAck() != 0)
  {
    goto cmd_error;
  }
  
  //step5,send out data
	vBspIicWriteByte(dat & 0x00ff);
  
  //wait ack
  if(vBspIicWaitAck() != 0)
  {
    goto cmd_error;
  }
  
  //step6,transfer complete,send stop singnal
  vBspIicSendStop();
  
  return 1;
  
cmd_error:
  vBspIicSendStop();
  return 0;
}

uint16_t usDrvOpt3001ReadByte(uint8_t ucRegAddr)
{
uint8_t ucResMsb = 0, ucResLsb = 0;
  
  //step1,send out start singnal,for send address and data
	vBspIicSendStart();
  
  //step2,send out device address,and set oparate direction
	vBspIicWriteByte((OPT3001_I2C_ADDRESS << 1) | IIC_WR); 
  
  //wait ack
  if(vBspIicWaitAck() != 0)
    goto cmd_error; 
  
  //step3,send out address
	vBspIicWriteByte(ucRegAddr); 
  
  //wait ack
  if(vBspIicWaitAck() != 0)
    goto cmd_error; 
  
  //step4,transfer complete,send stop singnal
  vBspIicSendStop();
  
  //step5,send out start singnal,for send data
	vBspIicSendStart();
  
  //step6,send out device address,and set oparate direction
	vBspIicWriteByte((OPT3001_I2C_ADDRESS << 1) | IIC_RD); 
  
  //wait ack
  if(vBspIicWaitAck() != 0)
    goto cmd_error; 
    
  ucResMsb = vBspIicReadByte(IIC_ACK);
  ucResLsb = vBspIicReadByte(IIC_NACK);
  
  //step7,transfer complete,send stop singnal
  vBspIicSendStop();
  
  return ((ucResMsb << 8) + ucResLsb);
  
cmd_error:
  vBspIicSendStop();
  return 0xff;
}

void vDrvOpt3001Init(void)
{
	vBspIicInit();
  
  if((usDrvOpt3001ReadByte(REG_MANUFACTURER_ID) == MANUFACTURER_ID) &&
     (usDrvOpt3001ReadByte(REG_DEVICE_ID) == DEVICE_ID)) {
    ucDrvOpt3001WriteByte(REG_CONFIGURATION, CONFIG_ENABLE);
  }
  else
  {
    // something was wrong
  }
}

// 扩大100倍返回，便于通信和转移数据
uint16_t usDrvOpt3001Sample(void)
{
static uint16_t usOpt3001ResReg = 0;
static uint16_t usOpt3001Factor = 0;  
static uint32_t ulOpt3001ResCov = 0;

  
  // 获取转换结果
  usOpt3001ResReg = usDrvOpt3001ReadByte(0x00);
  // 获取LSB SIZE
  usOpt3001Factor = (1 << ((usOpt3001ResReg & 0xf000) >> 12));
  // 计算强度值
  ulOpt3001ResCov = usOpt3001Factor * (usOpt3001ResReg & 0x0fff);
  
#if SYS_USING_LOG_OUTPUT
//    printf("\r\n 当前光照强度为：%.2f lux \r\n LSB size = %.2f lux \r\n", 
//           ((float)ulOpt3001ResCov) / 100.0, usOpt3001Factor * 0.01);
#endif // #if SYS_USING_LOG_OUTPUT
      
  return (ulOpt3001ResCov / 100);
}
