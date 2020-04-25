/**************************************************************************
  Company:
    Self.

  File Name:
    bsp_led.c

  Description:
    .
  **************************************************************************/

#ifndef __SYS_PLATFORM_CONFIG_H
#define __SYS_PLATFORM_CONFIG_H

#include "main.h"

// *****************************************************************************
// Section: 通用定义；
// *****************************************************************************

/*** 常用枚举 ******************************************************************/
typedef enum {PRIO_L0 = 0, PRIO_L1, PRIO_L2, PRIO_L3} xSysIntPriority_t;
typedef enum {USR_FALSE = 0, USR_TRUE = 1 } xUserBool_t;
typedef enum {OFF = 0, ON = 1 } xUserLevel_t;

/*** 常用宏定义 ****************************************************************/
#define USER_BIT_SET(_bit, flag)   (flag |=  (1 << _bit))
#define USER_BIT_CLR(_bit, flag)   (flag &= ~(1 << _bit))
#define USER_BIT_ASK(_bit, flag)   (flag & (0x01 << _bit))

#define USER_GET_MAX(a, b)          (((a) > (b)) ? (a) : (b))
#define USER_GET_MIN(a, b)          (((a) > (b)) ? (b) : (a))

#define USER_MAKE_HALF_WORD(a, b)   (((a) << 8) | (b))
#define USER_MAKE_WORD(a, b, c, d)  (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

/*** 日志输出以及断言管理 ********************************************************/
#define SYS_LOG_PRINT_UART          NULL
#ifdef USE_DEBUG
  extern void v_bsp_uart_printf(const char *fmt, ...);
  extern void v_assert_failed(char *pc_info, char *pc_file_name, uint32_t ul_line);
  #define USER_ASSERT(string, expr) \
    ((expr) ? (void)0 : v_assert_failed((char *)string, (char *)__FILE__, __LINE__))
  #define LOG_OUT_INF(fmt, arg...) \
    v_bsp_uart_printf("<<-SYS-INFO->> " fmt "\n", ##arg);
  #define LOG_OUT_ERR(fmt, arg...) \
    v_bsp_uart_printf("<<-SYS-ERROR->> " fmt "\n", ##arg);
  #define LOG_OUT_DBG(fmt, arg...) \
    v_bsp_uart_printf("<<-SYS-DEBUG->> [%d]" fmt "\n", __LINE__, ##arg);
  #define LOG_OUT_DAT(fmt, arg...) \
    v_bsp_uart_printf(fmt, ##arg);
#else
  #define USER_ASSERT(string, expr)
  #define LOG_OUT_INF(fmt, arg...)
  #define LOG_OUT_ERR(fmt, arg...)
  #define LOG_OUT_DBG(fmt, arg...)
  #define LOG_OUT_DAT(fmt, arg...)
#endif // #ifdef  USE_DEBUG

// *****************************************************************************
// Section: IAP配置
// *****************************************************************************

#define FLASH_APP_BOOTLOADER_ADDRESS  0x08000000
#define BOOTLOADER_MAX_PAGE_NUM       (11)
#define FLASH_USER_ADDRESS            0x08005800
#define FLASH_USER_MAX_PAGE_NUM       (1)
#define FLASH_APP_RUNING_ADDRESS      0x08006000
#define FLASH_APP_BACKUP_ADDRESS      0x08013000
#define APP_MAX_PAGE_NUM              (26)
#define APP_MAX_SIZE                  (APP_MAX_PAGE_NUM * FLASH_PAGE_SIZE)

// 标志位存放区域；
#define FLASH_IAP_APP_SIZE_ADDRESS    FLASH_USER_ADDRESS
#define FLASH_IAP_APP_SIZE_SIZE       (4)
#define FLASH_IAP_BACKUP_OK_ADDRESS   (FLASH_IAP_APP_SIZE_ADDRESS + FLASH_IAP_APP_SIZE_SIZE)
#define FLASH_IAP_BACKUP_OK_SIZE      (2)
#define FLASH_IAP_RUNING_OK_ADDRESS   (FLASH_IAP_BACKUP_OK_ADDRESS + FLASH_IAP_BACKUP_OK_SIZE)
#define FLASH_IAP_RUNING_OK_SIZE      (2)
#define FLASH_IAP_ASKIAP_OK_ADDRESS   (FLASH_IAP_RUNING_OK_ADDRESS + FLASH_IAP_RUNING_OK_SIZE)
#define FLASH_IAP_ASKIAP_OK_SIZE      (2)

// 标志位值；
#define FLASH_FLAG_BACKUP_OK_VALUE    0x5566
#define FLASH_FLAG_RUNING_OK_VALUE    0x8899
#define FLASH_FLAG_ASKIAP_OK_VALUE    0x1122
#define FLASH_FLAG_INVALID_VALUE      0xffff

#define SYS_USING_IO_IAP_TRIG         (0x01)

// *****************************************************************************
// Section: 外设配置
// *****************************************************************************

/*** 时钟 配置 *****************************************************************/
// systick中断频率；
#define   SYSTICK_INT_FREQ        1000     //1KHz
// 选择系统时钟源；
#define SYSTEM_USE_HSI48
//#define SYSTEM_USE_HSE
#ifdef SYS_USING_HSE
  // Value of the External oscillator in Hz
  #define HSE_VALUE                 ((uint32_t)8000000)
#endif

/*** IAP触发引脚 ***************************************************************/
#if (SYS_USING_IO_IAP_TRIG == 0x01)
// 管脚
#define DRV_IAP_TRIG_PIN            LL_GPIO_PIN_12
#define DRV_IAP_TRIG_PORT           GPIOA
#define DRV_IAP_TRIG_CLK            LL_AHB1_GRP1_PERIPH_GPIOA
#endif // #if (SYS_USING_IO_IAP_TRIG == 0x01)

/*** LED 配置 *****************************************************************/
// 管脚
#define DRV_IND_LED_PIN             LL_GPIO_PIN_12
#define DRV_IND_LED_PORT            GPIOB
#define DRV_IND_LED_CLK             LL_AHB1_GRP1_PERIPH_GPIOB

// led的数量以及编号；
// 如果需要增加或减少LED数量，请在该枚举内增加相应的编号；
// 并在数组 ** px_led_pin_use_table[] ** 按顺序增加或减少管脚配置信息；
typedef enum {
	LED_IND = 0,
  LED_NUM,
}xTableLedIndex_t;

/*** 串口配置 ******************************************************************/
#define SYSTEM_USING_USART1   0
#define SYSTEM_USING_USART2   1

#if SYSTEM_USING_USART1
  #define DRV_USART1_TX_PIN           LL_GPIO_PIN_9
  #define DRV_USART1_RX_PIN           LL_GPIO_PIN_10
  #define DRV_USART1_PORT             GPIOA
  #define DRV_USART1_PORT_CLK         LL_AHB1_GRP1_PERIPH_GPIOA

  #define DRV_USART1_SIMPLEX_PIN      LL_GPIO_PIN_11
  #define DRV_USART1_SIMPLEX_PORT     GPIOA
  #define DRV_USART1_SIMPLEX_PORT_CLK LL_AHB1_GRP1_PERIPH_GPIOA

  #define UART1_DMA_CH_TX             LL_DMA_CHANNEL_2
  #define UART1_DMA_CH_RX             LL_DMA_CHANNEL_3
  #define UART1_DMA_TX_RX_IRQ         DMA1_Channel2_3_IRQn

  #define BSP_USART1_TX_BUF_SZ        (16)
  #define BSP_USART1_RX_BUF_SZ        (64)
#endif // #if SYSTEM_USING_USART1

#if SYSTEM_USING_USART2
  #define DRV_USART2_TX_PIN           LL_GPIO_PIN_2
  #define DRV_USART2_RX_PIN           LL_GPIO_PIN_3
  #define DRV_USART2_PORT             GPIOA
  #define DRV_USART2_PORT_CLK         LL_AHB1_GRP1_PERIPH_GPIOA

  #define DRV_USART2_SIMPLEX_PIN      LL_GPIO_PIN_1
  #define DRV_USART2_SIMPLEX_PORT     GPIOA
  #define DRV_USART2_SIMPLEX_PORT_CLK LL_AHB1_GRP1_PERIPH_GPIOA

  #define UART2_DMA_CH_TX             LL_DMA_CHANNEL_4
  #define UART2_DMA_CH_RX             LL_DMA_CHANNEL_5
  #define UART2_DMA_TX_RX_IRQ         DMA1_Channel4_5_6_7_IRQn

  #define BSP_USART2_TX_BUF_SZ        (16)
  #define BSP_USART2_RX_BUF_SZ        (2048ul)
#endif // #if SYSTEM_USING_USART2

#endif // #ifndef __SYS_PLATFORM_CONFIG_H

/*******************************************************************************
 End of File
*/