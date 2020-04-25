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
#include "system_platform_config.h"

#include "f0_systick_delay.h"

#include "stdarg.h"
#include "stdio.h"
#include <string.h>

#include "sys_fifo.h"

// *****************************************************************************
// Section: Data Types.
// *****************************************************************************
typedef enum {
  USART_BAUD_002400 = 2400,
  USART_BAUD_004800 = 4800,
  USART_BAUD_009600 = 9600,
  USART_BAUD_038400 = 38400,
  USART_BAUD_057600 = 57600,
  USART_BAUD_115200 = 115200,
  USART_BAUD_230400 = 230400,
  USART_BAUD_460800 = 460800,
  USART_BAUD_921600 = 921600,
} BSP_USART_BAUD;

// 串口控制命令；
typedef enum {
  USART_CTRL_CMD_RXNE_INT_CLR = 0x01,
  USART_CTRL_CMD_RXNE_INT_SET = 0x02,
  USART_CTRL_CMD_TC_INT_CLR   = 0x04,
  USART_CTRL_CMD_TC_INT_SET   = 0x08,
  USART_CTRL_CMD_TXE_INT_SET  = 0x10,
  USART_CTRL_CMD_TXE_INT_CLR  = 0x20,
  USART_CTRL_CMD_IDLE_INT_SET = 0x40,
  USART_CTRL_CMD_IDLE_INT_CLR = 0x80,
} BSP_USART_CTRL_CMD;

// 串口事件；用于回调函数；
typedef enum {
  USART_EVENT_RXNE       = 0x01,
  USART_EVENT_TXE        = 0x02,
  USART_EVENT_RX_DMADONE = 0x03,
  USART_EVENT_TX_DMADONE = 0x04,
  USART_EVENT_RX_TIMEOUT = 0x05,
  USART_EVENT_TC         = 0x06,
  USART_EVENT_IDLE       = 0x07,
} BSP_USART_EVENT;

// 串口支持的开启模式；
typedef enum {
  USART_OPEN_POLLING     = 0x0001,
  USART_OPEN_INT_RXNE    = 0x0002,
  USART_OPEN_INT_TXE     = 0x0004,
  USART_OPEN_DMA_RX_BOOT = 0x0008, // 初始化完成后，马上打开DMA接收
  USART_OPEN_DMA_RX_WAIT = 0x0010, // 等待相关标志置位，才打开DMA接收
  USART_OPEN_DMA_TX      = 0x0020,
  USART_OPEN_INT_TC      = 0x0040,
  USART_OPEN_SIMPLEX     = 0x0080,
  USART_OPEN_INT_IDLE    = 0x0100,
  USART_OPEN_LIN_MODE    = 0x0200,
} BSP_USART_OPEN_MODE;

// 半双工通信方向；
typedef enum {
  SIMPLEX_SEND = 0,
  SIMPLEX_RECV,
} xTableUsartDir_t;

// 串口引脚管理结构体；
typedef struct {
  GPIO_TypeDef* px_rx_port;
  uint32_t      ul_rx_pin;
  uint32_t      ul_rx_pin_af;
  uint32_t      ul_rx_pin_clock_source;

  GPIO_TypeDef* px_tx_port;
  uint32_t      ul_tx_pin;
  uint32_t      ul_tx_pin_af;
  uint32_t      ul_tx_pin_clock_source;

  GPIO_TypeDef* px_dir_port;
  uint32_t      ul_dir_pin;
  uint32_t      ul_dir_pin_clock_source;
} xBspUsartPins_t;

// 串口初始化参数；
typedef struct {
  xSysIntPriority_t x_priority;   // 中断优先级；

  uint16_t          us_open_mode; // 串口开启模式；

  uint32_t          ul_dma_tx_mode;
  uint32_t          ul_dma_rx_mode;

  xBspUsartPins_t   x_pins;       // 管脚信息；
  LL_USART_InitTypeDef
      x_base_para; // 基础参数结构体，包含波特率、数据位、校验信息等；
} xBspUsartInitParas_t;

/* STM32 uart HANDLE */
typedef struct {
  USART_TypeDef* px_index;
  IRQn_Type      x_irq;
  IRQn_Type      x_irq_dma;

  xBspUsartInitParas_t* px_init_paras;

  DMA_TypeDef* px_tx_dma;
  uint32_t     ul_tx_dma_channel;
  DMA_TypeDef* px_rx_dma;
  uint32_t     ul_rx_dma_channel;

  xFifo_t* px_tx_buff;
  uint16_t us_tx_buf_size;

  xFifo_t* px_rx_buff;
  uint16_t us_rx_buf_size;

  uint16_t us_rx_buf_recover_counter;
} xBspUsartInstant_t;

#if SYSTEM_USING_USART1
extern xBspUsartInitParas_t bsp_usart1_init_paras;
extern xBspUsartInstant_t   bsp_instant_usart1;
#endif // #if BSP_USING_UART1

#if SYSTEM_USING_USART2
extern xBspUsartInitParas_t bsp_usart2_init_paras;
extern xBspUsartInstant_t   bsp_instant_usart2;
#endif // #if BSP_USING_UART2

#if SYSTEM_USING_USART3
extern xBspUsartInitParas_t bsp_usart3_init_paras;
extern xBspUsartInstant_t   bsp_instant_usart3;
#endif // #if BSP_USING_UART3

// *****************************************************************************
// Section: Interface export.
// *****************************************************************************
extern xUserBool_t x_bsp_usart_init(
    xBspUsartInstant_t* px_usart, xBspUsartInitParas_t* px_init_paras);

extern void v_bsp_usart_byte_write(
    xBspUsartInstant_t* px_usart, uint8_t uc_char);
extern char c_bsp_usart_byte_read(xBspUsartInstant_t* px_usart);

extern void v_bsp_simplex_direction_set(
    xBspUsartInstant_t* px_usart, xTableUsartDir_t x_dir);

extern void v_bsp_usart_dma_tx_restart(
    xBspUsartInstant_t* px_usart, uint16_t us_length);
extern void v_bsp_usart_dma_rx_restart(
    xBspUsartInstant_t* px_usart, uint8_t* puc_buffer, uint16_t us_length);

extern void v_bsp_usart_dma_rec_fifo_recover_counter(
    xBspUsartInstant_t* px_usart);
extern uint16_t us_bsp_usart_dma_rec_fifo_real_in_get(
    xBspUsartInstant_t* px_usart);

extern uint16_t us_bsp_usart_read(
    xBspUsartInstant_t* px_usart, uint8_t* puc_des_buffer, uint16_t us_length);
extern uint16_t us_bsp_usart_write(
    xBspUsartInstant_t* px_usart, uint8_t* puc_srs_buffer, uint16_t us_length);

#endif
