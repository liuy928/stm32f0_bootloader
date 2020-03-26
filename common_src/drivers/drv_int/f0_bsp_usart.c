/**************************************************************************
  Company:
    Self.

  File Name:
    bsp_led.c

  Description:
    .
  **************************************************************************/


// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************
#include "f0_bsp_usart.h"

// variables
/*** If use USART1 block, define the relevant variable, such as buffer, and driver handle. ***/
#if SYSTEM_USING_USART1
//usart1 driver handle
xBSPUsartHandle_t bspUsart1 = {
  .ID             = USART1,
  .irq            = USART1_IRQn,
  .irq_dma        = UART1_DMA_TX_RX_IRQ,
  //.priority;
  //.init;
  //.open_mode;

  .px_tx_dma          = DMA1,
  .ul_tx_dma_channel  = UART1_DMA_CH_TX,
  .px_rx_dma          = DMA1,
  .ul_rx_dma_channel  = UART1_DMA_CH_RX,

  .px_tx_buff       = NULL,
  .us_tx_buf_size    = BSP_USART1_TX_BUF_SZ,
  .px_rx_buff       = NULL,
  .us_rx_buf_size    = BSP_USART1_RX_BUF_SZ,
};

#endif // #if SYSTEM_USING_USART1

/*** If use USART2 block, define the relevant variable, such as buffer, and driver handle. ***/
#if SYSTEM_USING_USART2
//usart2 driver handle
xBSPUsartHandle_t bspUsart2 = {
  .ID             = USART2,
  .irq            = USART2_IRQn,
  .irq_dma        = UART2_DMA_TX_RX_IRQ,
  //.priority;
  //.init;
  //.open_mode;

  .px_tx_dma          = DMA1,
  .ul_tx_dma_channel  = UART2_DMA_CH_TX,
  .px_rx_dma          = DMA1,
  .ul_rx_dma_channel  = UART2_DMA_CH_RX,

  .px_tx_buff       = NULL,
  .us_tx_buf_size   = BSP_USART2_TX_BUF_SZ,
  .px_rx_buff       = NULL,
  .us_rx_buf_size   = BSP_USART2_RX_BUF_SZ,
};
#endif // #if SYSTEM_USING_USART2

/*** If use USART3 block, define the relevant variable, such as buffer, and driver handle. ***/
#if SYSTEM_USING_USART3
//usart2 driver handle
xBSPUsartHandle_t bspUsart3 = {
  .ID             = USART3,
  .irq            = USART3_4_IRQn,
  .irq_dma        = UART3_DMA_TX_RX_IRQ,
  //.priority;
  //.init;
  //.open_mode;

  .px_tx_dma          = DMA1,
  .ul_tx_dma_channel  = UART3_DMA_CH_TX,
  .px_rx_dma          = DMA1,
  .ul_rx_dma_channel  = UART3_DMA_CH_RX,

  .px_tx_buff       = NULL,
  .us_tx_buf_size   = BSP_USART3_TX_BUF_SZ,
  .px_rx_buff       = NULL,
  .us_rx_buf_size   = BSP_USART3_RX_BUF_SZ,
};
#endif // #if SYSTEM_USING_USART2

static void prvBspUsartHardwareSetup(xBSPUsartHandle_t* usart_index);
static void prvBspUsartPinsSetup(xBSPUsartHandle_t* usart_index);
static void prvBspUsartIntSetup(xBSPUsartHandle_t* usart_index);
static void prvBspUsartControl(xBSPUsartHandle_t* usart_index, BSP_USART_CTRL_CMD cmd);

static void prvUsartDmaTxConfig(xBSPUsartHandle_t* usart_index);
static void prvBspUsartDmaRxConfig(xBSPUsartHandle_t* usart_index);

static uint16_t prBspUsartPollingRead(xBSPUsartHandle_t* usart_index, void *destination, uint16_t nBytes);
static uint16_t prvBspUsartPollingWrite(xBSPUsartHandle_t* usart_index, void *destination, uint16_t nBytes);

static uint16_t prBspUsartInterruptRead(xBSPUsartHandle_t* usart_index, void *destination, uint16_t nBytes);
static uint16_t prvBspUsartInterruptWrite(xBSPUsartHandle_t* usart_index, void *destination, uint16_t nBytes);

static uint16_t prvBspUsartDmaRead(xBSPUsartHandle_t* usart_index, void *destination, uint16_t nBytes);

static void _bsp_usart_print_hex(xBSPUsartHandle_t* pxUsart, long long val);
static void _bsp_usart_print_dec(xBSPUsartHandle_t* pxUsart, long val);
static void _bsp_usart_print_float(xBSPUsartHandle_t* pxUsart, float val);

/* Pointer to Function:
    void bsp_usart_initialize(xBSPUsartHandle_t* usart_index, BSP_USART_INIT* init, SYS_INT_PRIORITY prio, uint8_t open_mode)

  Description:
    This function supports initialization a usart block by supplied usart index and initialization structor;
    The open mode detemine which mode will the usart exchange data;

  Parameters:
    - usart_index- the usart index;
    - init- the initialization parameters structor;
    - prio- interrupt priority;
    - open_mode - the work mode of the usart will be initialization;

  Returns:
    - None;
 */
void vBspUsartInit(xBSPUsartHandle_t* usart_index,
                   LL_USART_InitTypeDef* init,
                   SYS_INT_PRIORITY prio,
                   uint8_t open_mode)
{
  if(usart_index == NULL) { return; }
  if(init == NULL) { return; }

  usart_index->init = init;
  usart_index->priority = prio;
  usart_index->open_mode = open_mode;

  prvBspUsartPinsSetup(usart_index);

  /*---------------------------- FIFO���� -----------------------*/
  // ��������FIFO��
  // �Ƿ񴴽�ȡ���ڵ�ǰ��ģʽ�Ƿ���Ҫ�õ�FIFO
  if((usart_index->open_mode & BSP_USART_OPEN_DMA_TX) ||
     (usart_index->open_mode & BSP_USART_OPEN_INT_TXE) ||
     (usart_index->open_mode & BSP_USART_OPEN_INT_TC))
  {
    if(usart_index->px_tx_buff == NULL)
    {
      usart_index->px_tx_buff = px_fifo_create(usart_index->us_tx_buf_size);
    }
  }
  // ��������FIFO��
  // �Ƿ񴴽�ȡ���ڵ�ǰ��ģʽ�Ƿ���Ҫ�õ�FIFO
  if((usart_index->open_mode & BSP_USART_OPEN_DMA_RX_BOOT) ||
     (usart_index->open_mode & BSP_USART_OPEN_DMA_RX_WAIT) ||
     (usart_index->open_mode & BSP_USART_OPEN_INT_RXNE))
  {
    if(usart_index->px_rx_buff == NULL)
    usart_index->px_rx_buff = px_fifo_create(usart_index->us_rx_buf_size);
  }

  //����򿪽��շǿ��ж�
  if(usart_index->open_mode & BSP_USART_OPEN_INT_RXNE)
  {
    prvBspUsartControl(usart_index, BSP_USART_CTRL_CMD_RXNE_INT_SET);
  }

  if(usart_index->open_mode & BSP_USART_OPEN_INT_IDLE)
  {
    prvBspUsartControl(usart_index, BSP_USART_CTRL_CMD_IDLE_INT_SET);
  }

  //�����DMA����
  if(usart_index->open_mode & BSP_USART_OPEN_DMA_TX)
  {
    prvUsartDmaTxConfig(usart_index);
  }

  //�����DMA����
  if((usart_index->open_mode & BSP_USART_OPEN_DMA_RX_BOOT) ||
     (usart_index->open_mode & BSP_USART_OPEN_DMA_RX_WAIT))
  {
    prvBspUsartDmaRxConfig(usart_index);
  }

  //����򿪰�˫��ģʽ
	if(usart_index->open_mode & BSP_USART_OPEN_SIMPLEX)
  {
		vDrvSimplexDirectionSet(usart_index, SIMPLEX_RECV);
  }

  prvBspUsartIntSetup(usart_index);
  prvBspUsartHardwareSetup(usart_index);
}

void vDrvSimplexDirectionSet(xBSPUsartHandle_t* uart_index, uint8_t dir)
{
#if SYSTEM_USING_USART1
  if(uart_index == &bspUsart1)
  {
    if(dir == SIMPLEX_SEND){
      LL_GPIO_SetOutputPin(DRV_USART2_SIMPLEX_PORT, DRV_USART2_SIMPLEX_PIN);
    } else {
      LL_GPIO_ResetOutputPin(DRV_USART2_SIMPLEX_PORT, DRV_USART2_SIMPLEX_PIN);
    }
  }
#endif // #if SYSTEM_USING_USART1

#if SYSTEM_USING_USART2
  if(uart_index == &bspUsart2)
  {
    if(dir == SIMPLEX_SEND) {
      LL_GPIO_SetOutputPin(DRV_USART2_SIMPLEX_PORT, DRV_USART2_SIMPLEX_PIN);
    } else {
      LL_GPIO_ResetOutputPin(DRV_USART2_SIMPLEX_PORT, DRV_USART2_SIMPLEX_PIN);
    }
  }
#endif

#if SYSTEM_USING_USART3
  if(uart_index == &bspUsart3)
  {
    if(dir == SIMPLEX_SEND) {
      LL_GPIO_SetOutputPin(DRV_USART3_SIMPLEX_PORT, DRV_USART3_SIMPLEX_PIN);
    } else {
      LL_GPIO_ResetOutputPin(DRV_USART3_SIMPLEX_PORT, DRV_USART3_SIMPLEX_PIN);
    }
  }
#endif
}

/* Pointer to Function:
    uint16_t bsp_usart_read(xBSPUsartHandle_t* usart_index, void *destination, uint16_t nBytes)

  Description:
    This function supports application to read some bytes from usart;
    The open mode of usart will decision with way will be used, polling? DMA? Interrupt?

  Parameters:
    - usart_index- the usart index;
    - destination- pointer to the receive data where will be store;
    - nBytes- exception read numbers;

  Returns:
    - (nBytes - size)- the number actully read;
 */
uint16_t usBspUsartRead(xBSPUsartHandle_t* usart_index, void *destination, uint16_t nBytes)
{
  if(usart_index == NULL)
  {
    ;//process error.
    return 0;
  }

  if(nBytes == 0)return 0;

  if(usart_index->open_mode & BSP_USART_OPEN_INT_RXNE)
  {
    return prBspUsartInterruptRead(usart_index, destination, nBytes);
  }
  else if((usart_index->open_mode & BSP_USART_OPEN_DMA_RX_BOOT) ||
          (usart_index->open_mode & BSP_USART_OPEN_DMA_RX_WAIT))
  {
    return prvBspUsartDmaRead(usart_index, destination, nBytes);
  }

  return prBspUsartPollingRead(usart_index, destination, nBytes);
}

/* Pointer to Function:
    uint16_t bsp_usart_write(xBSPUsartHandle_t* usart_index, void *destination, uint16_t nBytes)

  Description:
    This function supports application to write some bytes from usart;
    The open mode of usart will decision with way will be used, polling? DMA? Interrupt?

  Parameters:
    - usart_index- the usart index;
    - destination- pointer to the data where will be send;
    - nBytes- exception write numbers;

  Returns:
    - (nBytes - size)- the number actully read;
 */
uint16_t usBspUsartWrite(xBSPUsartHandle_t* usart_index, void *destination, uint16_t nBytes)
{
  if(usart_index == NULL)
  {
    ;//process error.
    return 0;
  }

	if(nBytes == 0)return 0;

	if(usart_index->open_mode & BSP_USART_OPEN_SIMPLEX)
	{
		vDrvSimplexDirectionSet(usart_index, SIMPLEX_SEND);
		vSystemDelayXus(50);
	}

	if(usart_index->open_mode & BSP_USART_OPEN_INT_TXE)
	{
		return prvBspUsartInterruptWrite(usart_index, destination, nBytes);
	}
	else if(usart_index->open_mode & BSP_USART_OPEN_DMA_TX)
	{
    memcpy(usart_index->px_tx_buff->puc_buffer, destination, nBytes);
    vBspUsartDmaTxRestart(usart_index, nBytes);
    //USART_DMACmd(usart_index->ID, USART_DMAReq_Tx, ENABLE);

	  return nBytes;
	}

	return prvBspUsartPollingWrite(usart_index, destination, nBytes);
}

/*
********************************************************************************
* �Զ���printf������ʵ�ָ�ʽ�������
* ֧�ִ��ı���ʮ���ơ��ַ�����ʮ�����ơ�����������λС������
* ֧�ֱ�Ƿ��磺\r,\n,\t��
********************************************************************************
*/
void vBspUsartKprintf(const char *fmt,...)
{
	uint8_t temp_d = 0x0d;
	uint8_t temp_a = 0x0a;
  uint8_t temp_t = 0x09;
	uint8_t temp = 0x5c; // '\'

  va_list args;		    //����һ���������ø������ı���

	uint8_t *p, *str;

	va_start(args, fmt);//��ʼ��apָ���һ������

	for(p = (uint8_t*)fmt;*p;p++)
	{
		if(*p == 0x5c)//'\'  ����Ƿ�б�ܿ�ʼ������ת���ַ�
		{
			switch(*++p)
			{
			case 'r':
				usBspUsartWrite(SYS_LOG_PRINT_UART, &temp_d, 1);

				break;

			case 'n':
				usBspUsartWrite(SYS_LOG_PRINT_UART, &temp_a, 1);

				break;

			case 't':
        usBspUsartWrite(SYS_LOG_PRINT_UART, &temp_t, 1);

        break;

			default:
			  usBspUsartWrite(SYS_LOG_PRINT_UART, (uint8_t*)&temp, 1);
        usBspUsartWrite(SYS_LOG_PRINT_UART, p, 1);

				break;
			}
		}
		else if(*p == '%')//�����ʽ����ʼ
		{
			switch(*++p)
			{
			case 's': // �ַ���
      case 'S':
					//va_arg
         str = (uint8_t*)va_arg(args, char*);
         usBspUsartWrite(SYS_LOG_PRINT_UART, str, strlen((const char*)str));

				break;

			case 'd': // ʮ����
      case 'i':
      case 'D':
      case 'I':
				//ʹ�����������������ز����Լ��ƶ���������ָ����һ������������������
				_bsp_usart_print_dec(SYS_LOG_PRINT_UART, va_arg(args, long));

        break;

      case 'x': // ȫСдʮ������
      case 'X':
				_bsp_usart_print_hex(SYS_LOG_PRINT_UART, va_arg(args, long));

        break;

      case 'f': // ���������޶�ֻ������С������λ
      case 'F':
        _bsp_usart_print_float(SYS_LOG_PRINT_UART, (float)va_arg(args, double));

        break;

			default:
        usBspUsartWrite(SYS_LOG_PRINT_UART, (uint8_t*)"%", 1);
        usBspUsartWrite(SYS_LOG_PRINT_UART, p, 1);

			  break;
			}
		}
		else
		{
		  usBspUsartWrite(SYS_LOG_PRINT_UART, p, 1);
    }
	}
	va_end(args);
}

/* Pointer to Function:
    void bsp_usart_dma_tx_restart(xBSPUsartHandle_t* usart_index, uint16_t nBytes)

  Description:
    This function supports to start a new DMA transmitt event.

  Parameters:
    - usart_index- the usart index;
    - nBytes- exception transmitt numbers;

  Returns:
    - None;
 */
void vBspUsartDmaTxRestart(xBSPUsartHandle_t* usart_index, uint16_t nBytes)
{
    if(usart_index->ID == USART1)
  {
    LL_DMA_ClearFlag_HT2(usart_index->px_tx_dma);
    LL_DMA_ClearFlag_TC2 (usart_index->px_tx_dma);
    LL_DMA_ClearFlag_TE2(usart_index->px_tx_dma);
  }
  else if(usart_index->ID == USART2)
  {
    LL_DMA_ClearFlag_HT4(usart_index->px_tx_dma);
    LL_DMA_ClearFlag_TC4 (usart_index->px_tx_dma);
    LL_DMA_ClearFlag_TE4(usart_index->px_tx_dma);
  }

#if SYSTEM_USING_USART3
  else if(usart_index->ID == USART3)
  {
    LL_DMA_ClearFlag_HT6(usart_index->px_tx_dma);
    LL_DMA_ClearFlag_TC6 (usart_index->px_tx_dma);
    LL_DMA_ClearFlag_TE6(usart_index->px_tx_dma);
  }
#endif

  LL_DMA_DisableChannel(usart_index->px_tx_dma, usart_index->ul_tx_dma_channel);
  LL_DMA_SetDataLength(usart_index->px_tx_dma, usart_index->ul_tx_dma_channel, nBytes);
  LL_DMA_EnableChannel(usart_index->px_tx_dma, usart_index->ul_tx_dma_channel);
}

/* Pointer to Function:
    void bsp_usart_dma_rx_restart(xBSPUsartHandle_t* usart_index, uint16_t nBytes)

  Description:
    This function supports to start a new DMA receive event.

  Parameters:
    - usart_index- the usart index;
    - nBytes- exception receive numbers;

  Returns:
    - None;
 */
void vBspUsartDmaRxRestart(xBSPUsartHandle_t* usart_index, uint16_t nBytes)
{
  /* Clear DMA Transfer Flags and Open Clock for Speical DmaXStreanY */
  if(usart_index->ID == USART1)
  {
    /* Clear DMA Transfer Complete Flags */
    LL_DMA_ClearFlag_HT3(usart_index->px_rx_dma);
    LL_DMA_ClearFlag_TC3 (usart_index->px_rx_dma);
    LL_DMA_ClearFlag_TE3(usart_index->px_rx_dma);
  }
  else if(usart_index->ID == USART2)
  {
    /* Clear DMA Transfer Complete Flags */
    LL_DMA_ClearFlag_HT5(usart_index->px_rx_dma);
    LL_DMA_ClearFlag_TC5 (usart_index->px_rx_dma);
    LL_DMA_ClearFlag_TE5(usart_index->px_rx_dma);
  }

#if SYSTEM_USING_USART3
  else if(usart_index->ID == USART3)
  {
    /* Clear DMA Transfer Complete Flags */
    LL_DMA_ClearFlag_HT7(usart_index->px_rx_dma);
    LL_DMA_ClearFlag_TC7 (usart_index->px_rx_dma);
    LL_DMA_ClearFlag_TE7(usart_index->px_rx_dma);
  }
#endif

  LL_DMA_DisableChannel(usart_index->px_rx_dma, usart_index->ul_rx_dma_channel);
  LL_DMA_SetMemoryAddress(usart_index->px_rx_dma,
                          usart_index->ul_rx_dma_channel,
                          (uint32_t)usart_index->px_rx_buff->puc_buffer);
  LL_DMA_SetDataLength(usart_index->px_rx_dma, usart_index->ul_rx_dma_channel, nBytes);
  LL_DMA_EnableChannel(usart_index->px_rx_dma, usart_index->ul_rx_dma_channel);
}

/* Pointer to Function:
    static void _bsp_usart_hardware_setup(xBSPUsartHandle_t* usart_index)

  Description:
    This function supports using the giving usart handle which include initialization
    parameters structor.

    This is a local funciton, call by usart initialize function.

  Parameters:
    - usart_index- the usart index will be setup;

  Returns:
    None;
 */
static void prvBspUsartHardwareSetup(xBSPUsartHandle_t* usart_index)
{
  if(usart_index == NULL)
  {
    ;
    return;
  }

  LL_USART_Init(usart_index->ID, usart_index->init);
  // Perform basic configuration of USART for enabling use in Asynchronous Mode (UART)
  LL_USART_ConfigAsyncMode(usart_index->ID);
  /* Enable USART */
  LL_USART_Enable(usart_index->ID);
  /* reset USART_FLAG_TC, to prevent the putc function lost the first byte,
    after usart init*/
	LL_USART_ClearFlag_TC(usart_index->ID);
}

/* Pointer to Function:
    static void _bsp_usart_pins_setup(void)

  Description:
    This function supports config the pins and ports of usart block.

    This is a local funciton, call by usart initialize function.

  Parameters:
    None;

  Returns:
    None;
 */
static void prvBspUsartPinsSetup(xBSPUsartHandle_t* usart_index)
{
	LL_GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;

#if SYSTEM_USING_USART1
	if(usart_index->ID == USART1)
	{
		/* Enable UART GPIO clocks */
	  LL_AHB1_GRP1_EnableClock(DRV_USART1_CLK);
	  /* Enable UART clock */
	  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_USART1);

	  /* Configure USART Rx/tx PIN */
	  GPIO_InitStruct.Pin         = DRV_USART1_TX_PIN | DRV_USART1_RX_PIN;
	  GPIO_InitStruct.Mode        = LL_GPIO_MODE_ALTERNATE;
	  GPIO_InitStruct.OutputType  = LL_GPIO_OUTPUT_PUSHPULL;
	  GPIO_InitStruct.Pull        = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Alternate   = LL_GPIO_AF_1;
    LL_GPIO_Init(DRV_USART1_PORT, &GPIO_InitStruct);

    if(usart_index->open_mode & BSP_USART_OPEN_SIMPLEX)
		{
      LL_AHB1_GRP1_EnableClock(DRV_USART1_SIMPLEX_PORT_CLK);

			GPIO_InitStruct.Pin        =   DRV_USART1_SIMPLEX_PIN;
		  GPIO_InitStruct.Mode       =   LL_GPIO_MODE_OUTPUT;
		  GPIO_InitStruct.OutputType =   LL_GPIO_OUTPUT_PUSHPULL;
		  GPIO_InitStruct.Pull       =   LL_GPIO_PULL_UP;
		  LL_GPIO_Init(DRV_USART1_SIMPLEX_PORT, &GPIO_InitStruct);
		}
	}
#endif /* USING_UART1 */

#if SYSTEM_USING_USART2
	if(usart_index->ID == USART2)
	{
	  /* Enable UART GPIO clocks */
	  LL_AHB1_GRP1_EnableClock(DRV_USART2_CLK);
	  /* Enable UART clock */
	  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);

	  /* Configure USART Rx/tx PIN */
	  GPIO_InitStruct.Pin         = DRV_USART2_TX_PIN | DRV_USART2_RX_PIN;
	  GPIO_InitStruct.Mode        = LL_GPIO_MODE_ALTERNATE;
	  GPIO_InitStruct.OutputType  = LL_GPIO_OUTPUT_PUSHPULL;
	  GPIO_InitStruct.Pull        = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Alternate   = LL_GPIO_AF_1;
	  LL_GPIO_Init(DRV_USART2_PORT, &GPIO_InitStruct);

		if(usart_index->open_mode & BSP_USART_OPEN_SIMPLEX)
		{
      LL_AHB1_GRP1_EnableClock(DRV_USART2_SIMPLEX_PORT_CLK);

			GPIO_InitStruct.Pin        =   DRV_USART2_SIMPLEX_PIN;
		  GPIO_InitStruct.Mode       =   LL_GPIO_MODE_OUTPUT;
		  GPIO_InitStruct.OutputType =   LL_GPIO_OUTPUT_PUSHPULL;
		  GPIO_InitStruct.Pull       =   LL_GPIO_PULL_UP;
		  LL_GPIO_Init(DRV_USART2_SIMPLEX_PORT, &GPIO_InitStruct);
		}
	}
#endif /* USING_UART2 */

#if SYSTEM_USING_USART3
	if(usart_index->ID == USART3)
	{
	  /* Enable UART GPIO clocks */
    LL_AHB1_GRP1_EnableClock(DRV_USART3_CLK);
	  /* Enable UART clock */
	  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);

	  /* Configure USART Rx/tx PIN */
	  GPIO_InitStruct.Pin         = DRV_USART3_TX_PIN | DRV_USART3_RX_PIN;
	  GPIO_InitStruct.Mode        = LL_GPIO_MODE_ALTERNATE;
	  GPIO_InitStruct.OutputType  = LL_GPIO_OUTPUT_PUSHPULL;
	  GPIO_InitStruct.Pull        = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Alternate   = LL_GPIO_AF_1;
    LL_GPIO_Init(DRV_USART3_PORT, &GPIO_InitStruct);

		if(usart_index->open_mode & BSP_USART_OPEN_SIMPLEX)
		{
      LL_AHB1_GRP1_EnableClock(DRV_USART3_SIMPLEX_PORT_CLK);

			GPIO_InitStruct.Pin        =   DRV_USART3_SIMPLEX_PIN;
		  GPIO_InitStruct.Mode       =   LL_GPIO_MODE_OUTPUT;
		  GPIO_InitStruct.OutputType =   LL_GPIO_OUTPUT_PUSHPULL;
		  GPIO_InitStruct.Pull       =   LL_GPIO_PULL_UP;
		  LL_GPIO_Init(DRV_USART3_SIMPLEX_PORT, &GPIO_InitStruct);
		}
	}
#endif
}

/* Pointer to Function:
    static void _bsp_usart_int_sertup(xBSPUsartHandle_t* usart_index)

  Description:
    This function supports enable the interrupt sourse for usart bolck.
    Including DMA interrupt and EVENT interrupt.

    This is a local funciton, call by usart initialize function.

  Parameters:
    - usart_index- the usart index will be setup;

  Returns:
    None;
 */
static void prvBspUsartIntSetup(xBSPUsartHandle_t* usart_index)
{
  /* Enable the USART Global Interrupt*/
  if((usart_index->open_mode & BSP_USART_OPEN_INT_RXNE) ||
     (usart_index->open_mode & BSP_USART_OPEN_INT_TXE) ||
     (usart_index->open_mode & BSP_USART_OPEN_INT_TC) ||
     (usart_index->open_mode & BSP_USART_OPEN_INT_IDLE))
  {
    NVIC_SetPriority(usart_index->irq, usart_index->priority & 0X0f);
    NVIC_EnableIRQ(usart_index->irq);
  }

  /* Enable the USART DMA Interrupt */
  if((usart_index->open_mode & BSP_USART_OPEN_DMA_RX_BOOT) ||
     (usart_index->open_mode & BSP_USART_OPEN_DMA_RX_WAIT) ||
     (usart_index->open_mode & BSP_USART_OPEN_DMA_TX))
  {
    NVIC_SetPriority(usart_index->irq_dma, usart_index->priority & 0X0f);
    NVIC_EnableIRQ(usart_index->irq_dma);
  }
}

/* Pointer to Function:
    static void usart_dma_tx_config(xBSPUsartHandle_t* usart_index)

  Description:
    This function supports config usart dma transmitt.
    If the application open usart with mode DMA transmitt, this function should be call by
    usart initialize function.

  Parameters:
    - usart_index- the usart index will be setup;

  Returns:
    None;
 */
static void prvUsartDmaTxConfig(xBSPUsartHandle_t* usart_index)
{
  /* Clear DMA Transfer Flags and Open Clock for Speical DmaXStreanY */
  if(usart_index->ID == USART1)
  {
    LL_DMA_ClearFlag_HT2(usart_index->px_tx_dma);
    LL_DMA_ClearFlag_TC2 (usart_index->px_tx_dma);
    LL_DMA_ClearFlag_TE2(usart_index->px_tx_dma);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
  }
  else if(usart_index->ID == USART2)
  {
    LL_DMA_ClearFlag_HT4(usart_index->px_tx_dma);
    LL_DMA_ClearFlag_TC4 (usart_index->px_tx_dma);
    LL_DMA_ClearFlag_TE4(usart_index->px_tx_dma);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
  }

#if SYSTEM_USING_USART3
  else if(usart_index->ID == USART3)
  {
    LL_DMA_ClearFlag_HT6(usart_index->px_tx_dma);
    LL_DMA_ClearFlag_TC6 (usart_index->px_tx_dma);
    LL_DMA_ClearFlag_TE6(usart_index->px_tx_dma);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
  }
#endif

  /* Configure DMA Stream */
  LL_DMA_SetMemoryAddress(usart_index->px_tx_dma, usart_index->ul_tx_dma_channel,
                          (uint32_t)usart_index->px_tx_buff->puc_buffer);
  LL_DMA_SetPeriphAddress(usart_index->px_tx_dma, usart_index->ul_tx_dma_channel,
                          (uint32_t)usart_index->ID + 0x24);

  LL_DMA_SetDataLength(usart_index->px_tx_dma, usart_index->ul_tx_dma_channel, usart_index->us_tx_buf_size);

  LL_DMA_SetDataTransferDirection(usart_index->px_tx_dma, usart_index->ul_tx_dma_channel, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
  LL_DMA_SetChannelPriorityLevel(usart_index->px_tx_dma, usart_index->ul_tx_dma_channel, LL_DMA_PRIORITY_LOW);
  LL_DMA_SetMode(usart_index->px_tx_dma, usart_index->ul_tx_dma_channel, LL_DMA_MODE_NORMAL);
  LL_DMA_SetPeriphIncMode(usart_index->px_tx_dma, usart_index->ul_tx_dma_channel, LL_DMA_PERIPH_NOINCREMENT);
  LL_DMA_SetMemoryIncMode(usart_index->px_tx_dma, usart_index->ul_tx_dma_channel, LL_DMA_MEMORY_INCREMENT);
  LL_DMA_SetPeriphSize(usart_index->px_tx_dma, usart_index->ul_tx_dma_channel, LL_DMA_PDATAALIGN_BYTE);
  LL_DMA_SetMemorySize(usart_index->px_tx_dma, usart_index->ul_tx_dma_channel, LL_DMA_MDATAALIGN_BYTE);

  LL_DMA_EnableIT_TC(usart_index->px_tx_dma, usart_index->ul_tx_dma_channel);
  // LL_DMA_EnableChannel(usart_index->px_tx_dma, usart_index->ul_tx_dma_channel);
  LL_USART_EnableDMAReq_TX(usart_index->ID);
}

/* Pointer to Function:
    static void usart_dma_tx_config(xBSPUsartHandle_t* usart_index)

  Description:
    This function supports config usart dma receive.
    If the application open usart with mode DMA receive, this function should be call by
    usart initialize function.

  Parameters:
    - usart_index- the usart index will be setup;

  Returns:
    None;
 */
static void prvBspUsartDmaRxConfig(xBSPUsartHandle_t* usart_index)
{
  /* Clear DMA Transfer Flags and Open Clock for Speical DmaXStreanY */
  if(usart_index->ID == USART1)
  {
    LL_DMA_ClearFlag_HT3(usart_index->px_rx_dma);
    LL_DMA_ClearFlag_TC3 (usart_index->px_rx_dma);
    LL_DMA_ClearFlag_TE3(usart_index->px_rx_dma);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
  }
  else if(usart_index->ID == USART2)
  {
    LL_DMA_ClearFlag_HT5(usart_index->px_rx_dma);
    LL_DMA_ClearFlag_TC5 (usart_index->px_rx_dma);
    LL_DMA_ClearFlag_TE5(usart_index->px_rx_dma);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
  }

#if SYSTEM_USING_USART3
  else if(usart_index->ID == USART3)
  {
    LL_DMA_ClearFlag_HT7(usart_index->px_rx_dma);
    LL_DMA_ClearFlag_TC7 (usart_index->px_rx_dma);
    LL_DMA_ClearFlag_TE7(usart_index->px_rx_dma);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
  }
#endif

  /* Configure DMA Stream */
  LL_DMA_SetMemoryAddress(usart_index->px_rx_dma, usart_index->ul_rx_dma_channel,
                          (uint32_t)usart_index->px_rx_buff->puc_buffer);
  LL_DMA_SetPeriphAddress(usart_index->px_rx_dma, usart_index->ul_rx_dma_channel,
                          (uint32_t)usart_index->ID + 0x24);

  LL_DMA_SetDataLength(usart_index->px_rx_dma, usart_index->ul_rx_dma_channel, usart_index->us_rx_buf_size);

  LL_DMA_SetDataTransferDirection(usart_index->px_rx_dma,
                                  usart_index->ul_rx_dma_channel,
                                  LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
  LL_DMA_SetChannelPriorityLevel(usart_index->px_rx_dma,
                                 usart_index->ul_rx_dma_channel,
                                 LL_DMA_PRIORITY_LOW);
  LL_DMA_SetMode(usart_index->px_rx_dma,
                 usart_index->ul_rx_dma_channel,
                 LL_DMA_MODE_NORMAL);
  LL_DMA_SetPeriphIncMode(usart_index->px_rx_dma,
                          usart_index->ul_rx_dma_channel,
                          LL_DMA_PERIPH_NOINCREMENT);
  LL_DMA_SetMemoryIncMode(usart_index->px_rx_dma,
                          usart_index->ul_rx_dma_channel,
                          LL_DMA_MEMORY_INCREMENT);
  LL_DMA_SetPeriphSize(usart_index->px_rx_dma,
                       usart_index->ul_rx_dma_channel,
                       LL_DMA_PDATAALIGN_BYTE);
  LL_DMA_SetMemorySize(usart_index->px_rx_dma,
                       usart_index->ul_rx_dma_channel,
                       LL_DMA_MDATAALIGN_BYTE);

  LL_DMA_EnableIT_TC(usart_index->px_rx_dma, usart_index->ul_rx_dma_channel);
  LL_DMA_EnableChannel(usart_index->px_rx_dma, usart_index->ul_rx_dma_channel);
  LL_USART_EnableDMAReq_RX(usart_index->ID);
}

/* Pointer to Function:
    static void _bsp_usart_control(xBSPUsartHandle_t* usart_index, BSP_USART_CTRL_CMD cmd)

  Description:
    This function supports enable or disable different interrupt sourse about the
    serial port.
    If the application open usart with interrupt, this function should be call by
    usart initialize function.

  Parameters:
    - usart_index- the usart index will be setup;
    - cmd- the control cmd;

  Returns:
    None;
 */
static void prvBspUsartControl(xBSPUsartHandle_t* usart_index, BSP_USART_CTRL_CMD cmd)
{
  if(usart_index == NULL) { return;}

  switch(cmd)
  {
    //�رս��ܷǿ��ж�
  case BSP_USART_CTRL_CMD_RXNE_INT_CLR:
    LL_USART_DisableIT_RXNE(usart_index->ID);

    break;

    //�򿪽��ܷǿ��ж�
  case BSP_USART_CTRL_CMD_RXNE_INT_SET:
     LL_USART_IsEnabledIT_RXNE(usart_index->ID);

    break;

    //�رշ�������ж�
  case BSP_USART_CTRL_CMD_TC_INT_CLR:
    LL_USART_ClearFlag_TC(usart_index->ID);
    LL_USART_DisableIT_TC(usart_index->ID);

    break;

    //�򿪷�������ж�
  case BSP_USART_CTRL_CMD_TC_INT_SET:
    LL_USART_EnableIT_TC(usart_index->ID);

    break;

    //�رշ��Ϳ��ж�
  case BSP_USART_CTRL_CMD_TXE_INT_CLR:
    LL_USART_DisableIT_TXE(usart_index->ID);

    break;

    //�򿪷��Ϳ��ж�
  case BSP_USART_CTRL_CMD_TXE_INT_SET:
    LL_USART_EnableIT_TXE(usart_index->ID);

    break;

    //�رտ����ж�
  case BSP_USART_CTRL_CMD_IDLE_INT_CLR:
    LL_USART_DisableIT_IDLE(usart_index->ID);

    break;

    //�򿪿����ж�
  case BSP_USART_CTRL_CMD_IDLE_INT_SET:
    LL_USART_EnableIT_IDLE(usart_index->ID);

    break;
  }
}

/* Pointer to Function:
    void _bsp_usart_byte_write(xBSPUsartHandle_t* usart_index, uint8_t byte)

  Description:
    This function supports send byte via usart port.

  Parameters:
    - usart_index- the usart index;
    - byte- the data will be send out;

  Returns:
    None;
 */
void vBspUsartByteWrite(xBSPUsartHandle_t* usart_index, uint8_t byte)
{
  if(usart_index == NULL) { return;}

  LL_USART_TransmitData8(usart_index->ID, byte);
  while(!LL_USART_IsActiveFlag_TC(usart_index->ID));
}

/* Pointer to Function:
    char _bsp_usart_byte_read(xBSPUsartHandle_t* usart_index)

  Description:
    This function supports read byte via usart port.

  Parameters:
    - usart_index- the usart index;

  Returns:
    - byte- the data read from usart;
 */
char cBspUsartByteRead(xBSPUsartHandle_t* usart_index)
{
  char byte;

  if(usart_index == NULL) { return 0; }

  byte = (char)-1;
  if(LL_USART_IsActiveFlag_RXNE(usart_index->ID))
  {
    byte = LL_USART_ReceiveData8(usart_index->ID);
  }

  return byte;
}

/* Pointer to Function:
    static uint16_t _bsp_usart_polling_read(xBSPUsartHandle_t* usart_index, void *destination, uint16_t nBytes)

  Description:
    This function supports read some bytes via usart port with polling mode.

  Parameters:
    - usart_index- the usart index;
    - destination- pointer to the receive data where will be store;
    - nBytes- exception read numbers;

  Returns:
    - (nBytes - size)- the number actully read;
 */
static uint16_t prBspUsartPollingRead(xBSPUsartHandle_t* usart_index, void *destination, uint16_t nBytes)
{
  uint16_t size = nBytes;
  uint8_t *data;
  char byte = (char)-1;

  if(usart_index == NULL)
  {
    ;//process error.
    return 0;
  }

  data = (uint8_t*)destination;

  while(size)
  {
    byte = cBspUsartByteRead(usart_index);
    if(byte == (char)-1)break;

    *data = byte;
    data++; size--;
  }

  return (nBytes - size);
}

/* Pointer to Function:
    static uint16_t _bsp_usart_polling_write(xBSPUsartHandle_t* usart_index, void *destination, uint16_t nBytes)

  Description:
    This function supports write some bytes via usart port with polling mode.

  Parameters:
    - usart_index- the usart index;
    - destination- pointer to the data where will be send;
    - nBytes- exception write numbers;

  Returns:
    - (nBytes - size)- the number actully write;
 */
static uint16_t prvBspUsartPollingWrite(xBSPUsartHandle_t* usart_index, void *destination, uint16_t nBytes)
{
  uint16_t size = nBytes;
  uint8_t *data;

  if(usart_index == NULL)
  {
    ;//process error.
    return 0;
  }

  data = (uint8_t*)destination;

  while(size)
  {
    vBspUsartByteWrite(usart_index, *data);
    ++data; size--;
  }

  if(usart_index->open_mode && BSP_USART_OPEN_SIMPLEX)
  {
    vSystemDelayXus(50);
    vDrvSimplexDirectionSet(usart_index, SIMPLEX_RECV);
  }

  return nBytes - size;
}

/* Pointer to Function:
    static uint16_t _bsp_usart_interrupt_read(xBSPUsartHandle_t* usart_index, void *destination, uint16_t nBytes)

  Description:
    This function supports read some bytes from usart receive buffer and interrupt mode.

  Parameters:
    - usart_index- the usart index;
    - destination- pointer to the receive data where will be store;
    - nBytes- exception read numbers;

  Returns:
    - (nBytes - size)- the number actully read;
 */
static uint16_t prBspUsartInterruptRead(xBSPUsartHandle_t* usart_index, void *destination, uint16_t nBytes)
{
  uint8_t *data;

  if(usart_index == NULL) { return 0;}

  data = (uint8_t*)destination;

  return us_fifo_get(usart_index->px_rx_buff, data, nBytes);
}

/* Pointer to Function:
    static uint16_t _bsp_usart_interrupt_write(xBSPUsartHandle_t* usart_index, void *destination, uint16_t nBytes)

  Description:
    This function supports write some bytes into usart transmitt buffer and interrupt mode.

  Parameters:
    - usart_index- the usart index;
    - destination- pointer to the data where will be send;
    - nBytes- exception write numbers;

  Returns:
    - (nBytes - size)- the number actully write;
 */
static uint16_t prvBspUsartInterruptWrite(xBSPUsartHandle_t* usart_index, void *destination, uint16_t nBytes)
{
  uint16_t size;
  uint8_t *data;

  if(usart_index == NULL) { return 0;}

  data = (uint8_t*)destination;

  size = us_fifo_put(usart_index->px_tx_buff, data, nBytes);

  if(usart_index->px_tx_buff->us_size != 0) {
    prvBspUsartControl(usart_index, BSP_USART_CTRL_CMD_TXE_INT_SET);
  }

  return size;
}

static uint16_t prvBspUsartDmaRead(xBSPUsartHandle_t* usart_index, void *destination, uint16_t nBytes)
{
  int16_t s_length = -1;
  uint8_t *puc_p = destination;
  uint16_t us_len = nBytes;

  if(puc_p == NULL) {
    return s_length;
  }
  //�ر�DMA����ֹת�ƹ��̣����ݱ����������ݸ��ǣ�
  LL_DMA_DisableChannel(usart_index->px_rx_dma, usart_index->ul_rx_dma_channel);
  //��ȡ���ܵ������ݳ���
  s_length = usart_index->us_rx_buf_size -
             LL_DMA_GetDataLength(usart_index->px_rx_dma, usart_index->ul_rx_dma_channel);
  // �յ�����������Ҫ��ȡ��
  if(s_length < us_len) {
    us_len = s_length;
  }
  // ת������
  memcpy(puc_p, usart_index->px_rx_buff->puc_buffer, us_len);
  // ��������DMA���䣬������һ�����ݰ�
  vBspUsartDmaRxRestart(usart_index, usart_index->us_rx_buf_size);

  // �������ݳ���
  return us_len;
}

static void _bsp_usart_print_hex(xBSPUsartHandle_t* pxUsart, long long val)
{
uint8_t pucDigit[] = "0123456789abcdefg";
uint8_t pucString[16];
uint8_t ucCount = 0;

	do{
		pucString[ucCount++] = pucDigit[val % 16];
		val /= 16;
	}while(val > 0);

	if(ucCount % 2 != 0)
  {
    pucString[ucCount++] = '0';
  }

  usBspUsartWrite(pxUsart, (uint8_t*)"0x", 2);
	while(ucCount > 0)
	{
    usBspUsartWrite(pxUsart, &pucString[--ucCount], 1);
  }
}

/*
@usart print interger
*input:     val-->the data will be process
*output:    none
*/
static void _bsp_usart_print_dec(xBSPUsartHandle_t* pxUsart, long val)
{
	uint8_t s[16];
	int cnt = 0;
	uint8_t temp_minus = '-';

	if(val < 0)
	{
		usBspUsartWrite(pxUsart, &temp_minus, 1);
		val = -val;
	}

	do{
		s[cnt++] = val % 10 + 0x30;
		val /= 10;
	}while(val > 0);

	while(cnt > 0)
	{
	  usBspUsartWrite(pxUsart, &s[--cnt], 1);
  }
}

/*
@usart print float, ����С�������λ������������
*input:     val-->the data will be process
*output:    none
*/
static void _bsp_usart_print_float(xBSPUsartHandle_t* pxUsart, float val)
{
	uint8_t s[16];
	int cnt = 0;

	int val_temp = (int)(val * 1000);// ����С�������λ

	if(val_temp < 0)
	{
		usBspUsartWrite(pxUsart, (uint8_t*)"-", 1);
		val_temp = -val_temp;
	}

  // ��������
  if((val_temp % 10) >= 5)
  {
     val_temp += 10;
  }
  val_temp /= 10;

	do{
		s[cnt++] = val_temp % 10 + 0x30;
		val_temp /= 10;
    if(cnt == 2)// ����С����
    {
      s[cnt++] = '.';
    }
	}while(val_temp > 0);

	while(cnt > 0)
  {
		usBspUsartWrite(pxUsart, &s[--cnt], 1);
  }
}

/* Pointer to Function:
    void bsp_usart_isr(xBSPUsartHandle_t* usart_index, BSP_USART_EVENT event)

  Description:
    This function supports interrupt service funtion;
    Use event to process different interrupt;
    This function call by interrupt of usart

  Parameters:
    - event- the event case interrupt;

  Returns:
    - None;
 */
//void bsp_usart_isr(xBSPUsartHandle_t* usart_index, BSP_USART_EVENT event)
//{
//	char ch = (char)-1;
//
//  switch(event & 0xff)
//  {
//    case BSP_USART_EVENT_RXNE:
//    {
//			__disable_irq();
//
//      if(usart_index->rx_fifo->size < usart_index->rx_fifo->buf_sz)
//      {
//        ch = usart_index->ID->RDR & 0xff;
//        if(ch == (char)-1)break;
//
//        usart_index->rx_fifo->buffer[usart_index->rx_fifo->put_index] = ch;
//        usart_index->rx_fifo->put_index += 1;
//        usart_index->rx_fifo->size += 1;
//
//        if(usart_index->rx_fifo->put_index >= usart_index->rx_fifo->buf_sz)
//          usart_index->rx_fifo->put_index = 0;
//      }
//      /* if the next position is read index, discard this 'read char' */
//      else
//      {
//        usart_index->rx_fifo->get_index++;
//        usart_index->rx_fifo->size -= 1;
//
//        if(usart_index->rx_fifo->get_index >= usart_index->rx_fifo->buf_sz)
//          usart_index->rx_fifo->get_index = 0;
//      }
//
//      __enable_irq();
//
//      break;
//    }
//
//    case BSP_USART_EVENT_TXE:
//    {
//      __disable_irq();
//
//      if(usart_index->tx_fifo->put_index != usart_index->tx_fifo->get_index)//not empty
//      {
//        usart_index->ID->TDR = usart_index->tx_fifo->buffer[usart_index->tx_fifo->get_index];
//        usart_index->tx_fifo->get_index++;
//
//        if(usart_index->tx_fifo->get_index == usart_index->tx_fifo->buf_sz)
//          usart_index->tx_fifo->get_index = 0;
//      }
//      else//empty
//      {
//        _bsp_usart_control(usart_index, BSP_USART_CTRL_CMD_TXE_INT_CLR);
//
//				if(usart_index->open_mode && BSP_USART_OPEN_SIMPLEX)
//        {
//          systick_delay_xus(50);
//       		drv_simplex_direction(usart_index, SIMPLEX_RECV);
//        }
//      }
//
//      __enable_irq();
//
//      break;
//  }
//
//  case BSP_USART_EVENT_IDLE:
//
//    break;
//  }
//}


