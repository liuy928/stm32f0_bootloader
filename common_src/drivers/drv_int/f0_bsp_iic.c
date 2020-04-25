#include "f0_bsp_iic.h"
#include "f0_systick_delay.h"

#define IIC_SPD       (2)  //us
//iic initialize
void vBspIicInit(void)
{
	LL_GPIO_InitTypeDef  GPIO_InitStruct;

	LL_AHB1_GRP1_EnableClock(DRV_IIC_PORT_CLK);
	  
	GPIO_InitStruct.Speed         = LL_GPIO_SPEED_FREQ_MEDIUM;                    
	GPIO_InitStruct.Mode          = LL_GPIO_MODE_OUTPUT;   
        GPIO_InitStruct.OutputType    = LL_GPIO_OUTPUT_OPENDRAIN;
        GPIO_InitStruct.Pull          = LL_GPIO_PULL_NO;
        GPIO_InitStruct.Pin           = DRV_IIC_SCL_PIN | DRV_IIC_SDA_PIN;
	LL_GPIO_Init(DRV_IIC_PORT, &GPIO_InitStruct);
}

// IIC起始信号；
void vBspIicSendStart(void)
{
	IIC_SCL_OUT_SET(1);
	IIC_SDA_OUT_SET(1);
	IIC_SDA_OUT_SET(0);
	IIC_SCL_OUT_SET(0);
}

// IIC停止信号；
void vBspIicSendStop(void)
{
	IIC_SCL_OUT_SET(1); 
	IIC_SDA_OUT_SET(0);
	IIC_SDA_OUT_SET(1);
	IIC_SCL_OUT_SET(1);
}

// 主机IIC等待从机响应信号
// 时序：SCL为高电平期间，SDA出现低电平；
// 1:err 
// 0:success
uint8_t vBspIicWaitAck(void)
{
	static uint8_t iic_err_counter = 0;
		
	IIC_SDA_OUT_SET(1);
	IIC_SCL_OUT_SET(1);
	
	while(IIC_SDA_IS_SET())
	{
		iic_err_counter++;
		if(iic_err_counter > 10)
		{
			vBspIicSendStop();
			iic_err_counter = 0;
			
			return 1;
		}	
	}
	IIC_SCL_OUT_SET(0);	
    
	return 0;
}

// IIC主机发送ACK信号
// 信号时序：SCL为高电平期间，SDA出现低电平
static void prvBspIicSendAck(void)
{
  // 拉低数据线
	IIC_SDA_OUT_SET(0);
  // 拉低时钟线	
  IIC_SCL_OUT_SET(0);
  // 拉高时钟线
	IIC_SCL_OUT_SET(1);	
  // 拉低时钟线
	IIC_SCL_OUT_SET(0);
  // 拉高数据线，释放总线
  IIC_SDA_OUT_SET(1);
}

// IIC主机发送NACK信号
// 信号时序：SCL为高电平期间，SDA维持高电平
static void prvBspIicSendNack(void)
{
  // 拉低时钟线
  IIC_SCL_OUT_SET(0);
  // 拉高数据线
	IIC_SDA_OUT_SET(1);	
  // 拉高时钟线
	IIC_SCL_OUT_SET(1);	
  // 拉低时钟线
	IIC_SCL_OUT_SET(0);
}

//read byte
//ack = 1:send ack
//ack = 0:send nack
uint8_t vBspIicReadByte(uint8_t iic_ack_flag)
{
	uint8_t iic_temp = 0,i = 0;
	
	IIC_SCL_OUT_SET(0);

	for(i = 0;i < 8;i++)
	{
    // 拉低时钟线，解锁数据线，让从机改变SDA电平
    IIC_SCL_OUT_SET(0);
    // 锁定数据线，等待主机读取信号
    IIC_SCL_OUT_SET(1);
		
		iic_temp <<= 1;	
		if(IIC_SDA_IS_SET())
		{
      iic_temp |= 0x01;
    }
	}
  
  if(iic_ack_flag == IIC_ACK)
    prvBspIicSendAck();
  else
    prvBspIicSendNack();
	
	return 	iic_temp;
}

//send byte
void vBspIicWriteByte(uint8_t dat)
{
	uint8_t i = 0;
	
	IIC_SCL_OUT_SET(0);
	for(i = 0;i < 8;i++)
	{
		if((dat<<i) & 0x80)	
			IIC_SDA_OUT_SET(1);
		else 
			IIC_SDA_OUT_SET(0);

		IIC_SCL_OUT_SET(1);	
		IIC_SCL_OUT_SET(0);
	}
}

uint8_t vBspIicCheckDevice(uint8_t _addr)
{
	uint8_t ucAck;

	vBspIicSendStart();		

	vBspIicWriteByte(_addr | IIC_WR);
	ucAck = vBspIicWaitAck();	

	vBspIicSendStop();			

	return ucAck;
}


