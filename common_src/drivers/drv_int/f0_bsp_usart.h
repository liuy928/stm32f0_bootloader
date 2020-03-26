/**************************************************************************
  Company:
    Self.

  File Name:
    bsp_led.c

  Description:
    .
  **************************************************************************/

#ifndef __BSP_USART_H
#define __BSP_USART_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************
#include "f0_systick_delay.h"

#include "stdio.h"
#include "stdarg.h"
#include <string.h>

#include "sys_fifo.h"

// *****************************************************************************
// *****************************************************************************
// Section: Data Types.
// *****************************************************************************
// *****************************************************************************
typedef enum{
  BSP_USART_BAUD_002400 =     2400,
  BSP_USART_BAUD_004800 =     4800,
  BSP_USART_BAUD_009600 =     9600,
  BSP_USART_BAUD_038400 =     38400,
  BSP_USART_BAUD_057600 =     57600,
  BSP_USART_BAUD_115200 =     115200,
  BSP_USART_BAUD_230400 =     230400,
  BSP_USART_BAUD_460800 =     460800,
  BSP_USART_BAUD_921600 =     921600,
}BSP_USART_BAUD;

/*** macro define for usart control command ***/
typedef enum{
  BSP_USART_CTRL_CMD_RXNE_INT_CLR = 0x01,
  BSP_USART_CTRL_CMD_RXNE_INT_SET = 0x02,
  BSP_USART_CTRL_CMD_TC_INT_CLR   = 0x04,
  BSP_USART_CTRL_CMD_TC_INT_SET   = 0x08,
  BSP_USART_CTRL_CMD_TXE_INT_SET  = 0x10,
  BSP_USART_CTRL_CMD_TXE_INT_CLR  = 0x20,
  BSP_USART_CTRL_CMD_IDLE_INT_SET  = 0x40,
  BSP_USART_CTRL_CMD_IDLE_INT_CLR  = 0x80,
}BSP_USART_CTRL_CMD;

/*** macro define for usart event ***/
typedef enum{
  BSP_USART_EVENT_RXNE            = 0x01,
  BSP_USART_EVENT_TXE             = 0x02,
  BSP_USART_EVENT_RX_DMADONE      = 0x03,
  BSP_USART_EVENT_TX_DMADONE      = 0x04,
  BSP_USART_EVENT_RX_TIMEOUT      = 0x05,
  BSP_USART_EVENT_TC    	        = 0x06,
  BSP_USART_EVENT_IDLE            = 0x07,
}BSP_USART_EVENT;

/* macro define for usart open mode *******************/
typedef enum{
  BSP_USART_OPEN_POLLING					=	0x0000,
  BSP_USART_OPEN_INT_RXNE         = 0x0001,
  BSP_USART_OPEN_INT_TXE          = 0x0002,
  BSP_USART_OPEN_DMA_RX_BOOT      =	0x0004,  // 初始化完成后，马上打开DMA接收
  BSP_USART_OPEN_DMA_RX_WAIT      =	0x0008,  // 等待相关标志置位，才打开DMA接收
  BSP_USART_OPEN_DMA_TX           = 0x0010,
  BSP_USART_OPEN_INT_TC						=	0x0020,
  BSP_USART_OPEN_SIMPLEX					= 0x0040,
  BSP_USART_OPEN_INT_IDLE					= 0x0080,
}BSP_USART_OPEN_MODE;

typedef struct{
	/* software fifo */
	uint8_t *buffer;
	uint16_t put_index, get_index;
  uint16_t buf_sz;
  uint16_t size;
}BSP_USART_FIFO;

/* STM32 uart HANDLE */
typedef struct xBSP_USART_HANDLE
{
  USART_TypeDef         *ID;
  IRQn_Type             irq;
  IRQn_Type             irq_dma;

  SYS_INT_PRIORITY      priority;

  LL_USART_InitTypeDef  *init;
  uint8_t               open_mode;

  DMA_TypeDef           *px_tx_dma;
  uint32_t              ul_tx_dma_channel;
  DMA_TypeDef           *px_rx_dma;
  uint32_t              ul_rx_dma_channel;

  //FIFO
  xFifo_t               *px_tx_buff;
  uint16_t              us_tx_buf_size;
  xFifo_t               *px_rx_buff;
  uint16_t              us_rx_buf_size;
}xBSPUsartHandle_t;

//SIMPLEX direction control
#define SIMPLEX_SEND        0x01
#define SIMPLEX_RECV        0x00

//declar usart handle and parameters.
#if BSP_USING_UART1 | BSP_USING_UART2 | BSP_USING_UART3
extern struct BSP_USART_INIT USART_CONFIG_DEFAULT;
#endif // #if BSP_USING_UART1 | BSP_USING_UART2 | BSP_USING_UART3

#if SYSTEM_USING_USART1
extern xBSPUsartHandle_t bspUsart1;
#endif // #if BSP_USING_UART1

#if SYSTEM_USING_USART2
extern xBSPUsartHandle_t bspUsart2;
#endif // #if BSP_USING_UART2

#if SYSTEM_USING_USART3
extern xBSPUsartHandle_t bspUsart3;
#endif // #if BSP_USING_UART3


// *****************************************************************************
// *****************************************************************************
// Section: Interface export.
// *****************************************************************************
// *****************************************************************************
extern void vBspUsartInit(xBSPUsartHandle_t* usart_index,
                          LL_USART_InitTypeDef* init,
                          SYS_INT_PRIORITY prio,
                          uint8_t open_mode);

extern void vDrvSimplexDirectionSet(xBSPUsartHandle_t* uart_index, uint8_t dir);

extern void vBspUsartByteWrite(xBSPUsartHandle_t* usart_index, uint8_t byte);
extern char cBspUsartByteRead(xBSPUsartHandle_t* usart_index);

extern uint16_t usBspUsartRead(xBSPUsartHandle_t* usart_index, void *destination, uint16_t nBytes);
extern uint16_t usBspUsartWrite(xBSPUsartHandle_t* usart_index, void *destination, uint16_t nBytes);

extern void vBspUsartDmaTxRestart(xBSPUsartHandle_t* usart_index, uint16_t nBytes);
extern void vBspUsartDmaRxRestart(xBSPUsartHandle_t* usart_index, uint16_t nBytes);

extern void vBspUsartKprintf(const char *fmt,...);

// extern void bsp_usart_isr(BSP_USART_HANDLE* usart_index, BSP_USART_EVENT event);

#endif

