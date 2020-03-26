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

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f0xx_hal.h"
#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_utils.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_usart.h"
#include "stm32f0xx_ll_exti.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_usart.h"
#include "stm32f0xx_ll_dma.h"


typedef enum {kFalse = 0, kTrue = 1} boolean;

typedef enum{
  kPrioL0 = 0, kPrioL1, kPrioL2, kPrioL3,
}SYS_INT_PRIORITY;

#define _flag_set_bit(_bit, flag)         (flag |=  (1 << _bit))
#define _flag_clr_bit(_bit, flag)         (flag &= ~(1 << _bit))
#define _isBit_set(_bit, flag)            (flag & (0x01 << _bit))

#define USER_GET_MAX(a, b)                (((a) > (b)) ? (a) : (b))
#define USER_GET_MIN(a, b)                (((a) > (b)) ? (b) : (a))

#define USER_MAKE_HALF_WORD(a, b)         (((a) << 8) | (b))
#define USER_MAKE_WORD(a, b, c, d)        (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

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

//flash maping for iap
//IAP空间10个page，20KB;应用程序设置最大75个扇区,150KB;
#define FLASH_APP_RUNING_ADDRESS      0x08005000
#define APP_MAX_PAGE_NUM              75ul
#define APP_MAX_SIZE                  (APP_MAX_PAGE_NUM * FLASH_PAGE_SIZE)

// 外部flash；
// 扇区0，保留；
// app参数区域；扇区1；
#define STORAGE_IAP_FLAG_BASE_SECTOR  (1)
#define STORAGE_IAP_FLAG_START_ADDR   (0x001000)

#define FLASH_FLAG_BACKUP_OK_VALUE    0xaa
#define FLASH_FLAG_BACKUP_OK_ADDR     0x001001

#define FLASH_FLAG_RUNNIGN_OK_VALUE   0xbb
#define FLASH_FLAG_RUNNIGN_OK_ADDR    0x001002

#define FLASH_IAP_APP_SIZE_ADDR       0x001003 // 4字节

// app备份区域；扇区2~扇区39，共152K
#define STORAGE_IAP_BKP_BASE_SECTOR   (2)
#define STORAGE_IAP_BACKUP_START_ADDR (0x002000)

// *****************************************************************************
// Section: Pins Mapping
// *****************************************************************************
/*** Button driver pins map ***/
#define DRV_BTN_PIN                 LL_GPIO_PIN_6
#define DRV_BTN_PORT                GPIOA
#define DRV_BTN_CLK                 LL_AHB1_GRP1_PERIPH_GPIOA

/*** LED driver pins map ***/
#define DRV_LED3_PIN                LL_GPIO_PIN_12
#define DRV_LED3_PORT               GPIOB
#define DRV_LED3_CLK                LL_AHB1_GRP1_PERIPH_GPIOB

#ifdef USE_LED_ARRAY
#define DRV_LED4_PIN                LL_GPIO_PIN_1
#define DRV_LED4_PORT               GPIOB
#define DRV_LED4_CLK                LL_AHB1_GRP1_PERIPH_GPIOB

#define DRV_LED5_PIN                LL_GPIO_PIN_4
#define DRV_LED5_PORT               GPIOB
#define DRV_LED5_CLK                LL_AHB1_GRP1_PERIPH_GPIOB

#define DRV_LED6_PIN                LL_GPIO_PIN_5
#define DRV_LED6_PORT               GPIOB
#define DRV_LED6_CLK                LL_AHB1_GRP1_PERIPH_GPIOB
#endif

/*** usart1 driver pins map ***/
#define DRV_USART1_TX_PIN           LL_GPIO_PIN_9
#define DRV_USART1_RX_PIN           LL_GPIO_PIN_10
#define DRV_USART1_PORT             GPIOA
#define DRV_USART1_CLK              LL_AHB1_GRP1_PERIPH_GPIOA

#define DRV_USART1_SIMPLEX_PIN      (~LL_GPIO_PIN_ALL) // LL_GPIO_PIN_1
#define DRV_USART1_SIMPLEX_PORT     GPIOA
#define DRV_USART1_SIMPLEX_PORT_CLK LL_AHB1_GRP1_PERIPH_GPIOA

#define UART1_DMA_CH_TX             LL_DMA_CHANNEL_2
#define UART1_DMA_CH_RX             LL_DMA_CHANNEL_3
#define UART1_DMA_TX_RX_IRQ         DMA1_Channel2_3_IRQn

/*** usart2 driver pins map ***/
#define DRV_USART2_TX_PIN           LL_GPIO_PIN_2
#define DRV_USART2_RX_PIN           LL_GPIO_PIN_15
#define DRV_USART2_PORT             GPIOA
#define DRV_USART2_CLK              LL_AHB1_GRP1_PERIPH_GPIOA

#define DRV_USART2_SIMPLEX_PIN      (~LL_GPIO_PIN_ALL) // LL_GPIO_PIN_1
#define DRV_USART2_SIMPLEX_PORT     GPIOA
#define DRV_USART2_SIMPLEX_PORT_CLK LL_AHB1_GRP1_PERIPH_GPIOA

#define UART2_DMA_CH_TX             LL_DMA_CHANNEL_4
#define UART2_DMA_CH_RX             LL_DMA_CHANNEL_5
#define UART2_DMA_TX_RX_IRQ         DMA1_Channel4_5_6_7_IRQn

/*** usart3 driver pins map ***/
#define DRV_USART3_TX_PIN           LL_GPIO_PIN_10
#define DRV_USART3_RX_PIN           LL_GPIO_PIN_11
#define DRV_USART3_PORT             GPIOB
#define DRV_USART3_CLK              LL_AHB1_GRP1_PERIPH_GPIOB

#define DRV_USART3_SIMPLEX_PIN      (~LL_GPIO_PIN_ALL) // LL_GPIO_PIN_4
#define DRV_USART3_SIMPLEX_PORT     GPIOC
#define DRV_USART3_SIMPLEX_PORT_CLK LL_AHB1_GRP1_PERIPH_GPIOC

#define UART3_DMA_CH_TX             LL_DMA_CHANNEL_6
#define UART3_DMA_CH_RX             LL_DMA_CHANNEL_7
#define UART3_DMA_TX_RX_IRQ         DMA1_Channel4_5_6_7_IRQn

/*** iic pins map ***/
#define DRV_IIC_SCL_PIN             LL_GPIO_PIN_6
#define DRV_IIC_SDA_PIN             LL_GPIO_PIN_7
#define DRV_IIC_PORT                GPIOB
#define DRV_IIC_PORT_CLK            LL_AHB1_GRP1_PERIPH_GPIOB

/*** exti pins map ***/
#define DRV_EXTI_BTN_OFF_PIN        LL_GPIO_PIN_8
#define DRV_EXTI_BTN_PORT_CLK       RCC_AHBPeriph_GPIOB
#define DRV_EXTI_BTN_PORT           GPIOB

#define DRV_EXTI_LINE_BTN_OFF       EXTI_Line8
#define DRV_EXTI_BTN_OFF_SRS        EXTI_PinSource8
#define DRV_EXTI_CLK                RCC_APB2Periph_SYSCFG
#define DRV_EXTI_PORT_SRS           EXTI_PortSourceGPIOB

/*** spi pins map ***/
#define DRV_SPI1_MOSI_PIN           LL_GPIO_PIN_5
#define DRV_SPI1_MISO_PIN           LL_GPIO_PIN_4
#define DRV_SPI1_SCLK_PIN           LL_GPIO_PIN_3
#define DRV_SPI1_PORT               GPIOB
#define DRV_SPI1_PORT_CLK           LL_AHB1_GRP1_PERIPH_GPIOB
#define DRV_SPI1_CLK                LL_APB1_GRP2_PERIPH_SPI1

#define DRV_SPI2_MOSI_PIN           LL_GPIO_PIN_15 // LL_GPIO_AF_0
#define DRV_SPI2_MISO_PIN           LL_GPIO_PIN_14 // LL_GPIO_AF_0
#define DRV_SPI2_SCLK_PIN           LL_GPIO_PIN_13 // LL_GPIO_AF_0
//#define DRV_SPI2_SCLK_PIN           LL_GPIO_PIN_10 // LL_GPIO_AF_5
#define DRV_SPI2_PORT               GPIOB
#define DRV_SPI2_PORT_CLK           LL_AHB1_GRP1_PERIPH_GPIOB

#define DRV_W25_CS_PIN              LL_GPIO_PIN_10
#define DRV_W25_CS_PORT             GPIOB
#define DRV_W25_CS_PORT_CLK         LL_AHB1_GRP1_PERIPH_GPIOB

// *****************************************************************************
// *****************************************************************************
// Section: Driver Configuration
// *****************************************************************************
// *****************************************************************************
#define SYS_USING_LOG_OUTPUT          (1)
#if SYS_USING_LOG_OUTPUT
#define SYS_USING_LOG_OUTPUT_UART     (1)
#if SYS_USING_LOG_OUTPUT_UART
#define SYS_LOG_OUTPUT_UART  USART1
#define SYS_LOG_PRINT_UART    NULL // (&bspUsart2)
#endif  // #if SYS_USING_LOG_OUTPUT_UART
#endif  // #if SYS_USING_LOG_OUTPUT

/*** IAP ***/
#define SYSTEM_USING_IAP            0

#define SYSTEM_USE_HSI48
//#define SYSTEM_USE_HSE
#ifdef SYS_USING_HSE
  #define HSE_VALUE                 ((uint32_t)8000000) /*!< Value of the External oscillator in Hz */
#endif

/*** Systick Driver Configuration ***/
#define   SYSTICK_INT_FREQ        1000     //1KHz

/*** Usart Driver Configuration ***/
#define SYSTEM_USING_USART1   0
#if SYSTEM_USING_USART1
  #define BSP_USART1_TX_BUF_SZ      19
  #define BSP_USART1_RX_BUF_SZ      19
#endif

#define SYSTEM_USING_USART2   1
#if SYSTEM_USING_USART2
  #define BSP_USART2_TX_BUF_SZ      16
  #define BSP_USART2_RX_BUF_SZ      1029ul
#endif

#define SYSTEM_USING_USART3   0
#if SYSTEM_USING_USART3
  #define BSP_USART3_TX_BUF_SZ      16
  #define BSP_USART3_RX_BUF_SZ      1029ul
#endif

/*** Timer Driver Configuration ***/
#define SYSTEM_USING_TIM1     0
#define SYSTEM_USING_LED_TIMER_GROUP1     1
#define SYSTEM_USING_LED_TIMER_GROUP2     0

/*** W25Q ***/
#define W25Q_SPI              SPI2
#define W25X_CS_PORT          DRV_W25_CS_PORT
#define W25X_CS_PIN           DRV_W25_CS_PIN

/*** EXTERNAL RTC ***/
#define SYSTEM_USING_EXT_RTC  1

/*** EXTI Driver Configuretion ***/
#define SYSTEM_USING_EXTI         1

/*** ADC Driver Configuretion ***/
#define SYSTEM_USING_EXTI     1

/*** EXTI Driver Configuretion ***/
#define SYSTEM_USING_ADC      1

#endif // #ifndef __SYS_PLATFORM_CONFIG_H

/*******************************************************************************
 End of File
*/
