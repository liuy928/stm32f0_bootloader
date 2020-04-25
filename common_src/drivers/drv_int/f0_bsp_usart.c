/**************************************************************************
  Company:
    Self.

  File Name:
    bsp_led.c

  Description:
    .
  **************************************************************************/

// *****************************************************************************
// Section: File includes
// *****************************************************************************
#include "f0_bsp_usart.h"
#include "sys_fifo.h"

// *****************************************************************************
// Section: 变量定义；
// *****************************************************************************

#if SYSTEM_USING_USART1
// usart1 driver handle
xBspUsartInstant_t bsp_instant_usart1 = {
  .px_index  = USART1,
  .x_irq     = USART1_IRQn,
  .x_irq_dma = UART1_DMA_TX_RX_IRQ,

  .px_init_paras = NULL,

  .px_tx_dma         = DMA1,
  .ul_tx_dma_channel = UART1_DMA_CH_TX,
  .px_rx_dma         = DMA1,
  .ul_rx_dma_channel = UART1_DMA_CH_RX,

  .px_tx_buff     = NULL,
  .us_tx_buf_size = BSP_USART1_TX_BUF_SZ,
  .px_rx_buff     = NULL,
  .us_rx_buf_size = BSP_USART1_RX_BUF_SZ,
};

#endif // #if SYSTEM_USING_USART1

#if SYSTEM_USING_USART2
// usart2 driver handle
xBspUsartInstant_t bsp_instant_usart2 = {
  .px_index  = USART2,
  .x_irq     = USART2_IRQn,
  .x_irq_dma = UART2_DMA_TX_RX_IRQ,

  .px_init_paras = NULL,

  .px_tx_dma         = DMA1,
  .ul_tx_dma_channel = UART2_DMA_CH_TX,
  .px_rx_dma         = DMA1,
  .ul_rx_dma_channel = UART2_DMA_CH_RX,

  .px_tx_buff     = NULL,
  .us_tx_buf_size = BSP_USART2_TX_BUF_SZ,
  .px_rx_buff     = NULL,
  .us_rx_buf_size = BSP_USART2_RX_BUF_SZ,
};
#endif // #if SYSTEM_USING_USART2

#if SYSTEM_USING_USART3
// usart2 driver handle
xBspUsartInstant_t bsp_instant_usart3 = {
  .px_index  = USART3,
  .x_irq     = USART3_4_IRQn,
  .x_irq_dma = UART3_DMA_TX_RX_IRQ,

  .px_init_paras = NULL,

  .px_tx_dma         = DMA1,
  .ul_tx_dma_channel = UART3_DMA_CH_TX,
  .px_rx_dma         = DMA1,
  .ul_rx_dma_channel = UART3_DMA_CH_RX,

  .px_tx_buff     = NULL,
  .us_tx_buf_size = BSP_USART3_TX_BUF_SZ,
  .px_rx_buff     = NULL,
  .us_rx_buf_size = BSP_USART3_RX_BUF_SZ,
};
#endif // #if SYSTEM_USING_USART2

// *****************************************************************************
// Section: 函数声明；
// *****************************************************************************
static void prv_usart_dma_tx_config(xBspUsartInstant_t* px_usart);
static void prv_bsp_usart_dma_rx_config(xBspUsartInstant_t* px_usart);

static uint16_t prus_bsp_usart_polling_read(
    xBspUsartInstant_t* px_usart, uint8_t* puc_des_buffer, uint16_t us_length);
static uint16_t prus_bsp_usart_polling_write(
    xBspUsartInstant_t* px_usart, uint8_t* puc_srs_buffer, uint16_t us_length);

static uint16_t prus_bsp_usart_interrupt_read(
    xBspUsartInstant_t* px_usart, uint8_t* puc_des_buffer, uint16_t us_length);
static uint16_t prus_bsp_usart_interrupt_write(
    xBspUsartInstant_t* px_usart, uint8_t* puc_srs_buffer, uint16_t us_length);

static uint16_t prv_bsp_usart_dma_read(
    xBspUsartInstant_t* px_usart, uint8_t* puc_des_buffer, uint16_t us_length);

static void _bsp_usart_print_hex(xBspUsartInstant_t* pxUsart, long long val);
static void _bsp_usart_print_dec(xBspUsartInstant_t* pxUsart, long val);
static void _bsp_usart_print_float(xBspUsartInstant_t* pxUsart, float val);

// @function
// 按照给定的参数对串口进行初始化；包括：引脚初始化、中断管理、串口开启模式、底层；
// @para px_usart - 要被初始化的的串口句柄；
// @para px_init_paras - 指向串口初始化参数结构体；
// @return xUserBool_t - 初始化成功与否；
xUserBool_t x_bsp_usart_init(
    xBspUsartInstant_t* px_usart, xBspUsartInitParas_t* px_init_paras)
{
  // 参数检查；
  if ((px_usart == NULL) || (px_init_paras == NULL)) {
    LOG_OUT_ERR("串口参数错误，无法正常开启串口");

    return USR_FALSE;
  }

  // 绑定初始化参数表；
  px_usart->px_init_paras = px_init_paras;

  { // 初始化串口引脚；
    LL_GPIO_InitTypeDef GPIO_InitStruct;
    // 开启时钟；
    LL_AHB1_GRP1_EnableClock(px_init_paras->x_pins.ul_rx_pin_clock_source);
    LL_AHB1_GRP1_EnableClock(px_init_paras->x_pins.ul_tx_pin_clock_source);
    // 管脚配置；
    GPIO_InitStruct.Mode       = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull       = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_HIGH;
    // 接收引脚；
    GPIO_InitStruct.Pin       = px_init_paras->x_pins.ul_rx_pin;
    GPIO_InitStruct.Alternate = px_init_paras->x_pins.ul_rx_pin_af;
    LL_GPIO_Init(px_init_paras->x_pins.px_rx_port, &GPIO_InitStruct);
    // 发送引脚；
    GPIO_InitStruct.Pin       = px_init_paras->x_pins.ul_tx_pin;
    GPIO_InitStruct.Alternate = px_init_paras->x_pins.ul_tx_pin_af;
    LL_GPIO_Init(px_init_paras->x_pins.px_tx_port, &GPIO_InitStruct);
    // 配置半双工方向控制引脚；
    if(px_init_paras->us_open_mode & USART_OPEN_SIMPLEX) {
      // 开启时钟；
      LL_AHB1_GRP1_EnableClock(px_init_paras->x_pins.ul_dir_pin_clock_source);
      // 管脚配置；
      GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
      GPIO_InitStruct.Pin  = px_init_paras->x_pins.ul_dir_pin;
      LL_GPIO_Init(px_init_paras->x_pins.px_dir_port, &GPIO_InitStruct);
    }
  }

  { // 创建FIFO；
    // 创建发送FIFO，
    if ((px_usart->px_init_paras->us_open_mode & USART_OPEN_DMA_TX)
        || (px_usart->px_init_paras->us_open_mode & USART_OPEN_INT_TXE)
        || (px_usart->px_init_paras->us_open_mode & USART_OPEN_INT_TC)) {
      if (px_usart->px_tx_buff == NULL) {
        px_usart->px_tx_buff = px_fifo_create(px_usart->us_tx_buf_size);
        if (px_usart->px_tx_buff == NULL) {
          return USR_FALSE;
        }
      }
    }

    // 创建接收FIFO，
    if ((px_usart->px_init_paras->us_open_mode & USART_OPEN_DMA_RX_BOOT)
        || (px_usart->px_init_paras->us_open_mode & USART_OPEN_DMA_RX_WAIT)
        || (px_usart->px_init_paras->us_open_mode & USART_OPEN_INT_RXNE)) {
      if (px_usart->px_rx_buff == NULL) {
        px_usart->px_rx_buff = px_fifo_create(px_usart->us_rx_buf_size);
        if (px_usart->px_rx_buff == NULL) {
          return USR_FALSE;
        }
      }
    }
  }

  { // 开启串口时钟，否则寄存器会无法操作；
    if (px_usart->px_index == USART1) {
      LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_USART1);
    } else if (px_usart->px_index == USART2) {
      LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
    } else if (px_usart->px_index == USART3) {
      LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);
    }
  }

  { // 串口开启模式配置；
    // 如果打开接收非空中断
    if (px_usart->px_init_paras->us_open_mode & USART_OPEN_INT_RXNE) {
      LL_USART_IsEnabledIT_RXNE(px_usart->px_index);
    }
    // 如果开启空闲中断；
    if (px_usart->px_init_paras->us_open_mode & USART_OPEN_INT_IDLE) {
      LL_USART_EnableIT_IDLE(px_usart->px_index);
    }
    // 如果打开DMA发送
    if (px_usart->px_init_paras->us_open_mode & USART_OPEN_DMA_TX) {
      prv_usart_dma_tx_config(px_usart);
    }
    // 如果打开DMA接收
    if ((px_usart->px_init_paras->us_open_mode & USART_OPEN_DMA_RX_BOOT)
        || (px_usart->px_init_paras->us_open_mode & USART_OPEN_DMA_RX_WAIT)) {
      prv_bsp_usart_dma_rx_config(px_usart);
    }
    // 如果打开半双工模式，初始化后默认配置为接收；
    if (px_usart->px_init_paras->us_open_mode & USART_OPEN_SIMPLEX) {
      v_bsp_simplex_direction_set(px_usart, SIMPLEX_RECV);
    }
  }

  { // 中断初始化
    // 开启串口全局中断;
    if ((px_usart->px_init_paras->us_open_mode & USART_OPEN_INT_RXNE)
        || (px_usart->px_init_paras->us_open_mode & USART_OPEN_INT_TXE)
        || (px_usart->px_init_paras->us_open_mode & USART_OPEN_INT_TC)
        || (px_usart->px_init_paras->us_open_mode & USART_OPEN_INT_IDLE)) {
      NVIC_SetPriority(px_usart->x_irq, px_usart->px_init_paras->x_priority);
      NVIC_EnableIRQ(px_usart->x_irq);
    }
    // 开启DMA中断；
    if ((px_usart->px_init_paras->us_open_mode & USART_OPEN_DMA_RX_BOOT)
        || (px_usart->px_init_paras->us_open_mode & USART_OPEN_DMA_RX_WAIT)
        || (px_usart->px_init_paras->us_open_mode & USART_OPEN_DMA_TX)) {
      NVIC_SetPriority(
          px_usart->x_irq_dma, px_usart->px_init_paras->x_priority);
      NVIC_EnableIRQ(px_usart->x_irq_dma);
    }
  }

  { // 初始化串口模块；
    // 初始化底层参数；
    LL_USART_Init(px_usart->px_index, &px_init_paras->x_base_para);

    // 设置为异步传输；
    LL_USART_ConfigAsyncMode(px_usart->px_index);
    // 开启串口传输；
    LL_USART_Enable(px_usart->px_index);
    // 清除串口TC标志，避免在发送数据时第一个字节发不出去；
    // 导致该问题的原因是，开启串口时，硬件接口会发送一个探测字符，导致TC标志置位；
    LL_USART_ClearFlag_TC(px_usart->px_index);
  }

  return USR_TRUE;
}

// @function 通过串口发送一个字符；
// @para px_usart - 要发送数据的串口句柄；
// @para uc_char - 要发送的字符；
// @return 无；
void v_bsp_usart_byte_write(xBspUsartInstant_t* px_usart, uint8_t uc_char)
{
  if (px_usart == NULL) {
    return;
  }

  LL_USART_TransmitData8(px_usart->px_index, uc_char);
  while (!LL_USART_IsActiveFlag_TC(px_usart->px_index))
    ;
}

// @function 通过查询的方式从串口接收一个字符；
// @para px_usart - 要接收数据的串口句柄；
// @return char - 如果接收成功，返回接收到的字符；否则返回-1；
char c_bsp_usart_byte_read(xBspUsartInstant_t* px_usart)
{
  if (px_usart == NULL) {
    return 0;
  }

  char c_char = (char)-1;
  if (LL_USART_IsActiveFlag_RXNE(px_usart->px_index)) {
    c_char = LL_USART_ReceiveData8(px_usart->px_index);
  }

  return c_char;
}

// @function 设置半双工工作模式下的数据流向；
// @para px_usart - 要被设置的串口句柄；
// @para x_dir - 要设置的方向；
// @return 无；
void v_bsp_simplex_direction_set(
    xBspUsartInstant_t* px_usart, xTableUsartDir_t x_dir)
{
  if (x_dir == SIMPLEX_SEND) {
    LL_GPIO_SetOutputPin(px_usart->px_init_paras->x_pins.px_dir_port,
        px_usart->px_init_paras->x_pins.ul_dir_pin);
  } else {
    LL_GPIO_ResetOutputPin(px_usart->px_init_paras->x_pins.px_dir_port,
        px_usart->px_init_paras->x_pins.ul_dir_pin);
  }
}

// @function 重启DMA发送；
// @para px_usart - 要重启发送的串口句柄；
// @para us_length - 新一次传输的数据长度；
// @return 无；
void v_bsp_usart_dma_tx_restart(
    xBspUsartInstant_t* px_usart, uint16_t us_length)
{
  if (px_usart->px_index == USART1) {
    LL_DMA_ClearFlag_HT2(px_usart->px_tx_dma);
    LL_DMA_ClearFlag_TC2(px_usart->px_tx_dma);
    LL_DMA_ClearFlag_TE2(px_usart->px_tx_dma);
  } else if (px_usart->px_index == USART2) {
    LL_DMA_ClearFlag_HT4(px_usart->px_tx_dma);
    LL_DMA_ClearFlag_TC4(px_usart->px_tx_dma);
    LL_DMA_ClearFlag_TE4(px_usart->px_tx_dma);
  } else if (px_usart->px_index == USART3) {
    LL_DMA_ClearFlag_HT6(px_usart->px_tx_dma);
    LL_DMA_ClearFlag_TC6(px_usart->px_tx_dma);
    LL_DMA_ClearFlag_TE6(px_usart->px_tx_dma);
  }

  LL_DMA_DisableChannel(px_usart->px_tx_dma, px_usart->ul_tx_dma_channel);
  LL_DMA_SetDataLength(
      px_usart->px_tx_dma, px_usart->ul_tx_dma_channel, us_length);
  LL_DMA_EnableChannel(px_usart->px_tx_dma, px_usart->ul_tx_dma_channel);
}

// @function 重启DMA接收；
// @para px_usart - 要重启接收的串口句柄；
// @para puc_buffer - 如果该参数为NULL，则新一次接收的数据会放入串口本身的FIFO；
//                    如果该参数不为NULL，则新一次接收的数据会放入该指针指向的空间；
// @para us_length - 新一次传输的数据长度；
// @return 无；
void v_bsp_usart_dma_rx_restart(
    xBspUsartInstant_t* px_usart, uint8_t* puc_buffer, uint16_t us_length)
{
  // Clear DMA Transfer Flags and Open Clock for Speical DmaXStreanY;
  if (px_usart->px_index == USART1) {
    /* Clear DMA Transfer Complete Flags */
    LL_DMA_ClearFlag_HT3(px_usart->px_rx_dma);
    LL_DMA_ClearFlag_TC3(px_usart->px_rx_dma);
    LL_DMA_ClearFlag_TE3(px_usart->px_rx_dma);
  } else if (px_usart->px_index == USART2) {
    /* Clear DMA Transfer Complete Flags */
    LL_DMA_ClearFlag_HT5(px_usart->px_rx_dma);
    LL_DMA_ClearFlag_TC5(px_usart->px_rx_dma);
    LL_DMA_ClearFlag_TE5(px_usart->px_rx_dma);
  } else if (px_usart->px_index == USART3) {
    /* Clear DMA Transfer Complete Flags */
    LL_DMA_ClearFlag_HT7(px_usart->px_rx_dma);
    LL_DMA_ClearFlag_TC7(px_usart->px_rx_dma);
    LL_DMA_ClearFlag_TE7(px_usart->px_rx_dma);
  }

  LL_DMA_DisableChannel(px_usart->px_rx_dma, px_usart->ul_rx_dma_channel);

  if (puc_buffer == NULL) {
    LL_DMA_SetMemoryAddress(px_usart->px_rx_dma, px_usart->ul_rx_dma_channel,
        (uint32_t)px_usart->px_rx_buff->puc_buffer);
  } else {
    LL_DMA_SetMemoryAddress(
        px_usart->px_rx_dma, px_usart->ul_rx_dma_channel, (uint32_t)puc_buffer);
  }

  LL_DMA_SetDataLength(
      px_usart->px_rx_dma, px_usart->ul_rx_dma_channel, us_length);
  LL_DMA_EnableChannel(px_usart->px_rx_dma, px_usart->ul_rx_dma_channel);
}

// @function 统计DMA接收完成中断次数；便于更新fifo的写入索引；
// @para px_usart - 当前使用的串口的句柄；
// @return 无；
void v_bsp_usart_dma_rec_fifo_recover_counter(xBspUsartInstant_t* px_usart)
{
  px_usart->us_rx_buf_recover_counter++;
}

// @function 串口DMA接收方式下FIFO的真实写入索引获取；
//           该函数必须在新的读取缓存之前调用一次；或者后台开启一个线程实时更新；
// @para px_usart - 当前使用的串口的句柄；
// @return 无；
uint16_t us_bsp_usart_dma_rec_fifo_real_in_get(xBspUsartInstant_t* px_usart)
{
  px_usart->px_rx_buff->us_in
      = (px_usart->us_rx_buf_recover_counter + 1) * px_usart->us_rx_buf_size
      - LL_DMA_GetDataLength(px_usart->px_rx_dma, px_usart->ul_rx_dma_channel);
  // 如果发生自然数回绕；写入指针会自动归0，因此DMA接收buffer覆盖次数也需要归0；
  if (px_usart->px_rx_buff->us_in < px_usart->us_rx_buf_size) {
    px_usart->us_rx_buf_recover_counter = 0;
  }

  return px_usart->px_rx_buff->us_in;
}

// @function 接收端口会将串口数据按照打开的方式将数据放入FIFO；
//           该读取函数会从串口缓存读取数据，如果数据读取成功则返回读到的数据长度，否则返回0；
// @para px_usart - 要读取数据的串口句柄；
// @para puc_des_buffer - 指向读取出来的数据存放的内存空间；
// @para us_length - 想要读取的字节数；
// @return uint16_t - 如果数据读取成功则返回读到的数据长度，否则返回0；
uint16_t us_bsp_usart_read(
    xBspUsartInstant_t* px_usart, uint8_t* puc_des_buffer, uint16_t us_length)
{
  if ((px_usart == NULL) || (us_length == 0) || (puc_des_buffer == NULL)) {
    LOG_OUT_ERR("参数错误，无法正确读取数据");

    return 0;
  }

  if (px_usart->px_init_paras->us_open_mode & USART_OPEN_INT_RXNE) {
    return prus_bsp_usart_interrupt_read(px_usart, puc_des_buffer, us_length);
  } else if ((px_usart->px_init_paras->us_open_mode & USART_OPEN_DMA_RX_BOOT)
      || (px_usart->px_init_paras->us_open_mode & USART_OPEN_DMA_RX_WAIT)) {
    return prv_bsp_usart_dma_read(px_usart, puc_des_buffer, us_length);
  } else if (px_usart->px_init_paras->us_open_mode & USART_OPEN_POLLING) {
    return prus_bsp_usart_polling_read(px_usart, puc_des_buffer, us_length);
  }

  // 不会执行到这里；
  return 0;
}

// @function
// 应用层调用该函数将要发送的数据，按照打开的发送模式通过串口往外发送；
// @para px_usart - 要发送数据的串口句柄；
// @para puc_des_buffer - 指向要发送的数据存放的内存空间；
// @para us_length - 想要发送的字节数；
// @return uint16_t - 如果数据发送成功则返回发送的数据长度，否则返回0；
uint16_t us_bsp_usart_write(
    xBspUsartInstant_t* px_usart, uint8_t* puc_srs_buffer, uint16_t us_length)
{
  if ((px_usart == NULL) || (us_length == 0) || (puc_srs_buffer == NULL)) {
    LOG_OUT_ERR("参数错误，无法正确发送数据");

    return 0;
  }

  if (px_usart->px_init_paras->us_open_mode & USART_OPEN_INT_TXE) {
    return prus_bsp_usart_interrupt_write(px_usart, puc_srs_buffer, us_length);
  } else if (px_usart->px_init_paras->us_open_mode & USART_OPEN_DMA_TX) {
    memcpy(px_usart->px_tx_buff->puc_buffer, puc_srs_buffer, us_length);
    v_bsp_usart_dma_tx_restart(px_usart, us_length);

    return us_length;
  } else if (px_usart->px_init_paras->us_open_mode & USART_OPEN_POLLING) {
    return prus_bsp_usart_polling_write(px_usart, puc_srs_buffer, us_length);
  }

  // 不会执行到这里；
  return 0;
}

/*
********************************************************************************
* 自定义printf函数，实现格式化输出；
* 支持纯文本、十进制、字符串、十六进制、浮点数（两位小数）；
* 支持标记符如：\r,\n,\t；
********************************************************************************
*/
void vBspUsartKprintf(const char* fmt, ...)
{
  uint8_t temp_d = 0x0d;
  uint8_t temp_a = 0x0a;
  uint8_t temp_t = 0x09;
  uint8_t temp   = 0x5c; // '\'

  va_list args; //声明一个依次引用各参数的变量

  uint8_t *p, *str;

  va_start(args, fmt); //初始化ap指向第一个参数

  for (p = (uint8_t*)fmt; *p; p++) {
    if (*p == 0x5c) //'\'  如果是反斜杠开始，解析转义字符
    {
      switch (*++p) {
      case 'r':
        us_bsp_usart_write(SYS_LOG_PRINT_UART, &temp_d, 1);

        break;

      case 'n':
        us_bsp_usart_write(SYS_LOG_PRINT_UART, &temp_a, 1);

        break;

      case 't':
        us_bsp_usart_write(SYS_LOG_PRINT_UART, &temp_t, 1);

        break;

      default:
        us_bsp_usart_write(SYS_LOG_PRINT_UART, (uint8_t*)&temp, 1);
        us_bsp_usart_write(SYS_LOG_PRINT_UART, p, 1);

        break;
      }
    } else if (*p == '%') //如果格式符开始
    {
      switch (*++p) {
      case 's': // 字符串
      case 'S':
        // va_arg
        str = (uint8_t*)va_arg(args, char*);
        us_bsp_usart_write(SYS_LOG_PRINT_UART, str, strlen((const char*)str));

        break;

      case 'd': // 十进制
      case 'i':
      case 'D':
      case 'I':
        //使用类型名来决定返回参数以及移动步长，并指向下一个参数，这里是整形
        _bsp_usart_print_dec(SYS_LOG_PRINT_UART, va_arg(args, long));

        break;

      case 'x': // 全小写十六进制
      case 'X':
        _bsp_usart_print_hex(SYS_LOG_PRINT_UART, va_arg(args, long));

        break;

      case 'f': // 浮点数，限定只保留到小数后两位
      case 'F':
        _bsp_usart_print_float(SYS_LOG_PRINT_UART, (float)va_arg(args, double));

        break;

      default:
        us_bsp_usart_write(SYS_LOG_PRINT_UART, (uint8_t*)"%", 1);
        us_bsp_usart_write(SYS_LOG_PRINT_UART, p, 1);

        break;
      }
    } else {
      us_bsp_usart_write(SYS_LOG_PRINT_UART, p, 1);
    }
  }
  va_end(args);
}

// @function 配置串口DMA发送；
// @para px_usart - 要配置DMA发送的串口句柄；
// @return 无；
static void prv_usart_dma_tx_config(xBspUsartInstant_t* px_usart)
{
  // Clear DMA Transfer Flags and Open Clock for Speical DmaXStreanY;
  if (px_usart->px_index == USART1) {
    LL_DMA_ClearFlag_HT2(px_usart->px_tx_dma);
    LL_DMA_ClearFlag_TC2(px_usart->px_tx_dma);
    LL_DMA_ClearFlag_TE2(px_usart->px_tx_dma);

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
  } else if (px_usart->px_index == USART2) {
    LL_DMA_ClearFlag_HT4(px_usart->px_tx_dma);
    LL_DMA_ClearFlag_TC4(px_usart->px_tx_dma);
    LL_DMA_ClearFlag_TE4(px_usart->px_tx_dma);

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
  } else if (px_usart->px_index == USART3) {
    LL_DMA_ClearFlag_HT6(px_usart->px_tx_dma);
    LL_DMA_ClearFlag_TC6(px_usart->px_tx_dma);
    LL_DMA_ClearFlag_TE6(px_usart->px_tx_dma);

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
  }

  // Configure DMA channel;
  LL_DMA_SetMemoryAddress(px_usart->px_tx_dma, px_usart->ul_tx_dma_channel,
      (uint32_t)px_usart->px_tx_buff->puc_buffer);
  LL_DMA_SetPeriphAddress(px_usart->px_tx_dma, px_usart->ul_tx_dma_channel,
      (uint32_t)px_usart->px_index + 0x24);
  LL_DMA_SetDataLength(px_usart->px_tx_dma, px_usart->ul_tx_dma_channel,
      px_usart->us_tx_buf_size);
  LL_DMA_SetDataTransferDirection(px_usart->px_tx_dma, px_usart->ul_tx_dma_channel, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
  LL_DMA_SetChannelPriorityLevel(px_usart->px_tx_dma, px_usart->ul_tx_dma_channel, LL_DMA_PRIORITY_LOW);
  LL_DMA_SetMode(px_usart->px_tx_dma, px_usart->ul_tx_dma_channel, px_usart->px_init_paras->ul_dma_tx_mode);
  LL_DMA_SetPeriphIncMode(px_usart->px_tx_dma, px_usart->ul_tx_dma_channel,
      LL_DMA_PERIPH_NOINCREMENT);
  LL_DMA_SetMemoryIncMode(px_usart->px_tx_dma, px_usart->ul_tx_dma_channel,
      LL_DMA_MEMORY_INCREMENT);
  LL_DMA_SetPeriphSize(
      px_usart->px_tx_dma, px_usart->ul_tx_dma_channel, LL_DMA_PDATAALIGN_BYTE);
  LL_DMA_SetMemorySize(
      px_usart->px_tx_dma, px_usart->ul_tx_dma_channel, LL_DMA_MDATAALIGN_BYTE);

  LL_DMA_EnableIT_TC(px_usart->px_tx_dma, px_usart->ul_tx_dma_channel);
  LL_USART_EnableDMAReq_TX(px_usart->px_index);
}

// @function 配置串口DMA接收；
// @para px_usart - 要配置DMA接收的串口句柄；
// @return 无；
static void prv_bsp_usart_dma_rx_config(xBspUsartInstant_t* px_usart)
{
  // Clear DMA Transfer Flags and Open Clock for Speical DmaXStreanY;
  if (px_usart->px_index == USART1) {
    LL_DMA_ClearFlag_HT3(px_usart->px_rx_dma);
    LL_DMA_ClearFlag_TC3(px_usart->px_rx_dma);
    LL_DMA_ClearFlag_TE3(px_usart->px_rx_dma);

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
  } else if (px_usart->px_index == USART2) {
    LL_DMA_ClearFlag_HT5(px_usart->px_rx_dma);
    LL_DMA_ClearFlag_TC5(px_usart->px_rx_dma);
    LL_DMA_ClearFlag_TE5(px_usart->px_rx_dma);

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
  } else if (px_usart->px_index == USART3) {
    LL_DMA_ClearFlag_HT7(px_usart->px_rx_dma);
    LL_DMA_ClearFlag_TC7(px_usart->px_rx_dma);
    LL_DMA_ClearFlag_TE7(px_usart->px_rx_dma);

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
  }

  /* Configure DMA Stream */
  LL_DMA_SetMemoryAddress(px_usart->px_rx_dma, px_usart->ul_rx_dma_channel,
      (uint32_t)px_usart->px_rx_buff->puc_buffer);
  LL_DMA_SetPeriphAddress(px_usart->px_rx_dma, px_usart->ul_rx_dma_channel,
      (uint32_t)px_usart->px_index + 0x24);

  LL_DMA_SetDataLength(px_usart->px_rx_dma, px_usart->ul_rx_dma_channel,
      px_usart->us_rx_buf_size);

  LL_DMA_SetDataTransferDirection(px_usart->px_rx_dma,
      px_usart->ul_rx_dma_channel, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
  LL_DMA_SetChannelPriorityLevel(
      px_usart->px_rx_dma, px_usart->ul_rx_dma_channel, LL_DMA_PRIORITY_LOW);
  LL_DMA_SetMode(px_usart->px_rx_dma, px_usart->ul_rx_dma_channel,
      px_usart->px_init_paras->ul_dma_rx_mode);
  LL_DMA_SetPeriphIncMode(px_usart->px_rx_dma, px_usart->ul_rx_dma_channel,
      LL_DMA_PERIPH_NOINCREMENT);
  LL_DMA_SetMemoryIncMode(px_usart->px_rx_dma, px_usart->ul_rx_dma_channel,
      LL_DMA_MEMORY_INCREMENT);
  LL_DMA_SetPeriphSize(
      px_usart->px_rx_dma, px_usart->ul_rx_dma_channel, LL_DMA_PDATAALIGN_BYTE);
  LL_DMA_SetMemorySize(
      px_usart->px_rx_dma, px_usart->ul_rx_dma_channel, LL_DMA_MDATAALIGN_BYTE);

  LL_USART_EnableDMAReq_RX(px_usart->px_index);
  LL_DMA_EnableIT_TC(px_usart->px_rx_dma, px_usart->ul_rx_dma_channel);
  LL_DMA_EnableChannel(px_usart->px_rx_dma, px_usart->ul_rx_dma_channel);
}

// @function 采用查询的方式从串口接收一定长度的数据；
// @para px_usart - 要读取数据的串口句柄；
// @para puc_des_buffer - 指向读取出来的数据存放的内存空间；
// @para us_length - 想要读取的字节数；
// @return uint16_t - 如果数据读取成功则返回读到的数据长度；
static uint16_t prus_bsp_usart_polling_read(
    xBspUsartInstant_t* px_usart, uint8_t* puc_des_buffer, uint16_t us_length)
{
  if (px_usart == NULL) {
    ; // process error.
    return 0;
  }

  uint16_t us_size = us_length;

  char     c_char         = (char)-1;
  uint8_t* puc_local_data = puc_des_buffer;

  while (us_size) {
    c_char = c_bsp_usart_byte_read(px_usart);
    if (c_char == (char)-1) {
      break;
    }

    *puc_local_data = c_char;
    puc_local_data++;
    us_size--;
  }

  return (us_length - us_size);
}

// @function 采用查询的方式通过串口发送一定长度的数据；
// @para px_usart - 要发送数据的串口句柄；
// @para puc_des_buffer - 指向要发送的数据存放的内存空间；
// @para us_length - 想要发送的字节数；
// @return uint16_t - 如果数据发送成功则返回发送的数据长度，否则返回0；
static uint16_t prus_bsp_usart_polling_write(
    xBspUsartInstant_t* px_usart, uint8_t* puc_srs_buffer, uint16_t us_length)
{
  if (px_usart == NULL) {
    ; // process error.
    return 0;
  }

  uint16_t us_size        = us_length;
  uint8_t* puc_local_data = puc_srs_buffer;

  // 如果串口工作在半双工模式，则设置为发送；
  if (px_usart->px_init_paras->us_open_mode & USART_OPEN_SIMPLEX) {
    v_bsp_simplex_direction_set(px_usart, SIMPLEX_SEND);
    vSystemDelayXus(50);
  }

  while (us_size) {
    v_bsp_usart_byte_write(px_usart, *puc_local_data);
    ++puc_local_data;
    us_size--;
  }

  // 如果开启了半双工模式，数据发送完成后，设置为接受；
  if (px_usart->px_init_paras->us_open_mode & USART_OPEN_SIMPLEX) {
    vSystemDelayXus(50);
    v_bsp_simplex_direction_set(px_usart, SIMPLEX_RECV);
  }

  return us_length - us_size;
}

// @function 采用中断的方式从串口接收一定长度的数据；
//           中断服务函数会将接收到的数据放入缓存，该函数从缓存里面读取一段数据；
// @para px_usart - 要读取数据的串口句柄；
// @para puc_des_buffer - 指向读取出来的数据存放的内存空间；
// @para us_length - 想要读取的字节数；
// @return uint16_t - 如果数据读取成功则返回读到的数据长度；
static uint16_t prus_bsp_usart_interrupt_read(
    xBspUsartInstant_t* px_usart, uint8_t* puc_des_buffer, uint16_t us_length)
{
  if (px_usart == NULL) {
    return 0;
  }

  return us_fifo_get(px_usart->px_rx_buff, puc_des_buffer, us_length);
}

// @function 采用中断的方式通过串口发送一定长度的数据；
//           该函数会将数据放入缓存，并触发发送中断，中断服务函数会负责将所有的数据均发送出去；
// @para px_usart - 要发送数据的串口句柄；
// @para puc_des_buffer - 指向要发送的数据存放的内存空间；
// @para us_length - 想要发送的字节数；
// @return uint16_t - 如果数据发送成功则返回发送的数据长度，否则返回0；
static uint16_t prus_bsp_usart_interrupt_write(
    xBspUsartInstant_t* px_usart, uint8_t* puc_srs_buffer, uint16_t us_length)
{
  if (px_usart == NULL) {
    return 0;
  }

  us_fifo_put(px_usart->px_tx_buff, puc_srs_buffer, us_length);

  // 启动发送；<TODO>
  //  if(vRingBufferIsEmpty(px_usart->px_tx_buff) ==
  //  BufferNotEmpty)//(pxUsart->tx_fifo->put_index !=
  //  pxUsart->tx_fifo->get_index)
  //  {
  //    prvBspUsartControl(px_usart, BSP_USART_CTRL_CMD_TXE_INT_SET);
  //  }

  return 0;
}

// @function 采用DMA的方式从串口接收一定长度的数据；
//           通常配合空闲中断使用，空闲中断发生后表示串口已经收到了一定长度的数据并放入了缓存，
//           该函数会将缓存内的数据转移并重启接收；
// @para px_usart - 要读取数据的串口句柄；
// @para puc_des_buffer - 指向读取出来的数据存放的内存空间；
// @para us_length - 想要读取的字节数；
// @return uint16_t - 如果数据读取成功则返回读到的数据长度；
static uint16_t prv_bsp_usart_dma_read(
    xBspUsartInstant_t* px_usart, uint8_t* puc_des_buffer, uint16_t us_length)
{

  if ((puc_des_buffer == NULL) || (px_usart == NULL)) {
    return 0;
  }

  return us_fifo_get(px_usart->px_rx_buff, puc_des_buffer, us_length);
}

static void _bsp_usart_print_hex(xBspUsartInstant_t* pxUsart, long long val)
{
  uint8_t pucDigit[] = "0123456789abcdefg";
  uint8_t pucString[16];
  uint8_t ucCount = 0;

  do {
    pucString[ucCount++] = pucDigit[val % 16];
    val /= 16;
  } while (val > 0);

  if (ucCount % 2 != 0) {
    pucString[ucCount++] = '0';
  }

  us_bsp_usart_write(pxUsart, (uint8_t*)"0x", 2);
  while (ucCount > 0) {
    us_bsp_usart_write(pxUsart, &pucString[--ucCount], 1);
  }
}

static void _bsp_usart_print_dec(xBspUsartInstant_t* pxUsart, long val)
{
  uint8_t s[16];
  int     cnt        = 0;
  uint8_t temp_minus = '-';

  if (val < 0) {
    us_bsp_usart_write(pxUsart, &temp_minus, 1);
    val = -val;
  }

  do {
    s[cnt++] = val % 10 + 0x30;
    val /= 10;
  } while (val > 0);

  while (cnt > 0) {
    us_bsp_usart_write(pxUsart, &s[--cnt], 1);
  }
}

static void _bsp_usart_print_float(xBspUsartInstant_t* pxUsart, float val)
{
  uint8_t s[16];
  int     cnt = 0;

  int val_temp = (int)(val * 1000); // 保留小数点后三位

  if (val_temp < 0) {
    us_bsp_usart_write(pxUsart, (uint8_t*)"-", 1);
    val_temp = -val_temp;
  }

  // 四舍五入
  if ((val_temp % 10) >= 5) {
    val_temp += 10;
  }
  val_temp /= 10;

  do {
    s[cnt++] = val_temp % 10 + 0x30;
    val_temp /= 10;
    if (cnt == 2) // 加载小数点
    {
      s[cnt++] = '.';
    }
  } while (val_temp > 0);

  while (cnt > 0) {
    us_bsp_usart_write(pxUsart, &s[--cnt], 1);
  }
}
