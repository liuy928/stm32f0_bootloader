#include "f0_bsp_iic.h"
#include "f0_systick_delay.h"

#define IIC_SPD       (2)  //us
//iic initialize
void vBspIicInit(void)
{
	LL_GPIO_InitTypeDef  GPIO_InitStruct;

	LL_AHB1_GRP1_EnableClock(DRV_IIC_PORT_CLK);
	  
	GPIO_InitStruct.Speed         = LL_GPIO_SPEED_FREQ_LOW;                    
	GPIO_InitStruct.Mode          = LL_GPIO_MODE_OUTPUT;   
  GPIO_InitStruct.OutputType    = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull          = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Pin           = DRV_IIC_SCL_PIN | DRV_IIC_SDA_PIN;
	LL_GPIO_Init(DRV_IIC_PORT, &GPIO_InitStruct);
}

// IIC起始信号；
// 时序：SCL为高电平时，SDA从高电平向低电平切换；
void vBspIicSendStart(void)
{
	IIC_SDA_OUT_SET(1);	//pull up data line, wait drop level
	IIC_SCL_OUT_SET(1);	//pull up clock line
	vSystemDelayXus(5);
	IIC_SDA_OUT_SET(0);
	vSystemDelayXus(5);
	IIC_SCL_OUT_SET(0);	//lock clock line
}

// IIC停止信号；
// 时序：SCL为高点平时，SDA从低电平向高电平切换；
void vBspIicSendStop(void)
{
	IIC_SDA_OUT_SET(0);	//pull down data line, wait raise level		
	IIC_SCL_OUT_SET(1);	//pull up clock line
	vSystemDelayXus(5);
	IIC_SDA_OUT_SET(1);
  vSystemDelayXus(5);
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
	vSystemDelayXus(IIC_SPD);	
	
	while(IIC_SDA_IS_SET())
	{
    vSystemDelayXus(IIC_SPD);
		iic_err_counter++;
		if(iic_err_counter > 50)
		{
			vBspIicSendStop();
			iic_err_counter = 0;
			
			return 1;
		}	
	}
	vSystemDelayXus(IIC_SPD);
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
	vSystemDelayXus(2);
  // 拉高时钟线
	IIC_SCL_OUT_SET(1);	
	vSystemDelayXus(2);
  // 拉低时钟线
	IIC_SCL_OUT_SET(0);
  vSystemDelayXus(2);
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
	vSystemDelayXus(2);
  // 拉高时钟线
	IIC_SCL_OUT_SET(1);	
	vSystemDelayXus(2);
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
	vSystemDelayXus(IIC_SPD);

	for(i = 0;i < 8;i++)
	{
    // 拉低时钟线，解锁数据线，让从机改变SDA电平
    IIC_SCL_OUT_SET(0);
		vSystemDelayXus(IIC_SPD);
    // 锁定数据线，等待主机读取信号
    IIC_SCL_OUT_SET(1);
		
		iic_temp <<= 1;	
		if(IIC_SDA_IS_SET())
		{
      iic_temp |= 0x01;
    }
    vSystemDelayXus(IIC_SPD);	
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
	vSystemDelayXus(IIC_SPD);
	
	for(i = 0;i < 8;i++)
	{
		if((dat<<i) & 0x80)	
			IIC_SDA_OUT_SET(1);
		else 
			IIC_SDA_OUT_SET(0);
		vSystemDelayXus(IIC_SPD);
		
    // 锁定数据线，等待从机读取信号
		IIC_SCL_OUT_SET(1);
		vSystemDelayXus(IIC_SPD);
    // 解锁数据线，等待主机改变SDA电平
		IIC_SCL_OUT_SET(0);
		vSystemDelayXus(IIC_SPD);
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


