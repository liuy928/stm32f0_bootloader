
// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "system_platform_config.h"

#include "system_poll_timer.h"
#include "system_task.h"

#include "ymodem.h"

#include "f0_bsp_flash.h"
#include "f0_bsp_led.h"
#include "f0_bsp_spi.h"
#include "f0_bsp_usart.h"

#include "drv_spi_flash.h"

#if SYS_USING_LOG_OUTPUT
#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE* f)
#endif /* __GNUC__ */

// PUTCHAR_PROTOTYPE
//{
//  /* Place your implementation of fputc here */
//  /* e.g. write a character to the USART */
//  USART_SendData(SYS_LOG_OUTPUT_UART, (uint8_t) ch);
//  /* Loop until the end of transmission */
//  while (USART_GetFlagStatus(SYS_LOG_OUTPUT_UART, USART_FLAG_TC) == RESET);
//  return ch;
//}
#endif // #if SYS_USING_LOG_OUTPUT

xBspUsartInitParas_t bsp_usart2_init_paras = { .x_priority = PRIO_L3,
  .us_open_mode
  = USART_OPEN_DMA_RX_BOOT | USART_OPEN_INT_IDLE | USART_OPEN_POLLING | USART_OPEN_SIMPLEX,

  .ul_dma_tx_mode = LL_DMA_MODE_NORMAL,
  .ul_dma_rx_mode = LL_DMA_MODE_CIRCULAR,

  {
      .px_rx_port = DRV_USART2_PORT,
      .ul_rx_pin = DRV_USART2_RX_PIN,
      .ul_rx_pin_af = GPIO_AF1_USART2,
      .ul_rx_pin_clock_source = DRV_USART2_PORT_CLK,

      .px_tx_port = DRV_USART2_PORT,
      .ul_tx_pin = DRV_USART2_TX_PIN,
      .ul_tx_pin_af = GPIO_AF1_USART2,
      .ul_tx_pin_clock_source = DRV_USART2_PORT_CLK,

      .px_dir_port = DRV_USART2_SIMPLEX_PORT,
      .ul_dir_pin = DRV_USART2_SIMPLEX_PIN,
      .ul_dir_pin_clock_source = DRV_USART2_SIMPLEX_PORT_CLK,
  },

  {
      .BaudRate = USART_BAUD_115200,
      .DataWidth = LL_USART_DATAWIDTH_8B,
      .StopBits = LL_USART_STOPBITS_1,
      .Parity = LL_USART_PARITY_NONE,
      .TransferDirection = LL_USART_DIRECTION_TX_RX,
      .HardwareFlowControl = LL_USART_HWCONTROL_NONE,
      .OverSampling = LL_USART_OVERSAMPLING_16,
  } };

// led引脚配置表；
xLedPins_t px_led_pin_use_table[LED_NUM] = {
  {
      DRV_IND_LED_PORT,
      DRV_IND_LED_PIN,
      DRV_IND_LED_CLK,
  },
};

typedef void (*pFunction)(void);
pFunction Jump_To_Application;

// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************

int main(void)
{
  /* Initialize all modules, including applic ation(s). */
  v_system_clock_init();
  v_bsp_led_init();

  uint16_t us_flag = FLASH_FLAG_INVALID_VALUE;
  // 读取是否有IAP请求的标志；如果有IAP请标志有效则进入IAP过程；
  // 这边与应用程序在有IAP请求时，可以设置该标志位后，重启系统后触发IAP过程；
  bsp_flash_read_n_hword(
      FLASH_IAP_ASKIAP_OK_ADDRESS, (void*)&us_flag, FLASH_IAP_ASKIAP_OK_SIZE / 2);
  if (us_flag == FLASH_FLAG_ASKIAP_OK_VALUE) {
    goto label_running_iap;
  }

#if (SYS_USING_IO_IAP_TRIG == 0x01)
  LL_GPIO_InitTypeDef GPIO_InitStruct;

  LL_AHB1_GRP1_EnableClock(DRV_IAP_TRIG_CLK);

  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  GPIO_InitStruct.Pin = DRV_IAP_TRIG_PIN;
  LL_GPIO_Init(DRV_IAP_TRIG_PORT, &GPIO_InitStruct);

  // 读取I/O触发IAP的电平状态；
  // 上电后如果检测到按键按下超过2s，进入IAP模式；
  if (!LL_GPIO_IsInputPinSet(DRV_IAP_TRIG_PORT, DRV_IAP_TRIG_PIN)) {
    vSystemDelayXms(2000);
    if (!LL_GPIO_IsInputPinSet(DRV_IAP_TRIG_PORT, DRV_IAP_TRIG_PIN)) {
      goto label_running_iap;
    }
  }
#endif // #if (SYS_USING_IO_IAP_TRIG == 0x01)

  // 固件备份区检测，是否有有效的备份固件需要转移；
  bsp_flash_read_n_hword(
      FLASH_IAP_BACKUP_OK_ADDRESS, (void*)&us_flag, FLASH_IAP_BACKUP_OK_SIZE / 2);
  if (us_flag != FLASH_FLAG_BACKUP_OK_VALUE) {
    // 备份区不存在有效固件，则直接跳转到运行区是否包含有效固件的检测过程；
    goto label_check_running;
  }

  // 读取被分去固件字节数；
  uint32_t ul_bin_size = 0;
  bsp_flash_read_n_hword(
      FLASH_IAP_APP_SIZE_ADDRESS, (void*)&ul_bin_size, FLASH_IAP_APP_SIZE_SIZE / 2);
  if (ul_bin_size >= APP_MAX_SIZE) {
    // 写入备份区无效的标志；
    us_flag = FLASH_FLAG_INVALID_VALUE;
    bsp_flash_write_n_hword_check(
        FLASH_IAP_BACKUP_OK_ADDRESS, (void*)&us_flag, FLASH_IAP_BACKUP_OK_SIZE / 2);

    // 备份区固件过大，则直接跳转到运行区是否包含有效固件的检测过程；
    goto label_check_running;
  }

  // 统计内部flash容纳固件需要的扇区数；
  uint8_t uc_needed_sector_nbr = ul_bin_size / FLASH_PAGE_SIZE;
  if ((uc_needed_sector_nbr % FLASH_PAGE_SIZE) != 0) {
    uc_needed_sector_nbr += 1;
  }

  // 准备开始转移固件，写入运行区无效的标志；
  us_flag = FLASH_FLAG_INVALID_VALUE;
  bsp_flash_write_n_hword_check(
      FLASH_IAP_RUNING_OK_ADDRESS, (void*)&us_flag, FLASH_IAP_RUNING_OK_SIZE / 2);

  // 擦除运行区扇区；
  bsp_flash_erase_npage(FLASH_APP_RUNING_ADDRESS, uc_needed_sector_nbr);

  // 转移固件；
  uint32_t ul_bin_moved_size = 0;
  uint32_t ul_backup_read_addr = FLASH_APP_BACKUP_ADDRESS;
  uint32_t ul_runing_prog_addr = FLASH_APP_RUNING_ADDRESS;
  uint8_t pus_move_temp_buffer[FLASH_PAGE_SIZE];
  while (ul_bin_moved_size < ul_bin_size) {
    v_bsp_led_toggle(LED_IND);

    bsp_flash_read_n_hword(
        ul_backup_read_addr, (void*)&pus_move_temp_buffer[0], FLASH_PAGE_SIZE / 2);
    bsp_flash_write_n_hword_nocheck(
        ul_runing_prog_addr, (void*)&pus_move_temp_buffer[0], FLASH_PAGE_SIZE / 2);

    ul_bin_moved_size += FLASH_PAGE_SIZE;
    ul_backup_read_addr += FLASH_PAGE_SIZE;
    ul_runing_prog_addr += FLASH_PAGE_SIZE;
  }

  // 是否转移完成；
  if (ul_bin_moved_size >= ul_bin_size) {
    // 转移固件完成，写入运行区有效的标志；
    us_flag = FLASH_FLAG_RUNING_OK_VALUE;
    bsp_flash_write_n_hword_check(
        FLASH_IAP_RUNING_OK_ADDRESS, (void*)&us_flag, FLASH_IAP_RUNING_OK_SIZE / 2);

    // 写入备份区无效的标志；
    us_flag = FLASH_FLAG_INVALID_VALUE;
    bsp_flash_write_n_hword_check(
        FLASH_IAP_BACKUP_OK_ADDRESS, (void*)&us_flag, FLASH_IAP_BACKUP_OK_SIZE / 2);
  }

label_check_running:

  bsp_flash_read_n_hword(FLASH_IAP_RUNING_OK_ADDRESS, (void*)&us_flag, FLASH_IAP_RUNING_OK_SIZE / 2);
  if (us_flag == FLASH_FLAG_RUNING_OK_VALUE) {
    // TODO 这个判断需要依据实际的RAM空间进行匹配修改；
    if (((*(__IO uint32_t*)FLASH_APP_RUNING_ADDRESS) & 0x2FFE0000) == 0x20000000) {
      // __set_PRIMASK(1);
      //跳转至用户代码
      Jump_To_Application = (pFunction)(*(__IO uint32_t*)(FLASH_APP_RUNING_ADDRESS + 4));
      //初始化用户程序的堆栈指针
      __set_MSP (*(__IO uint32_t*)FLASH_APP_RUNING_ADDRESS);
      Jump_To_Application();
    }
  }

label_running_iap:

  x_bsp_usart_init(&bsp_instant_usart2, &bsp_usart2_init_paras);

  while (1) {
    // 周期性扫描等待ymoderm接收到有效固件；
    if (x_ymodem_receive(&ul_bin_size) == ERR_OK) {
      // 保存bin文件大小
      bsp_flash_write_n_hword_check(
          FLASH_IAP_APP_SIZE_ADDRESS, (void*)&ul_bin_size, FLASH_IAP_APP_SIZE_SIZE / 2);

      // 写入备份区有效的标志；
      us_flag = FLASH_FLAG_BACKUP_OK_VALUE;
      bsp_flash_write_n_hword_check(
          FLASH_IAP_BACKUP_OK_ADDRESS, (void*)&us_flag, FLASH_IAP_BACKUP_OK_SIZE / 2);

      vSystemDelayXms(500);
      //重启系统；
      NVIC_SystemReset();
    }
  }
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @param  None
 * @retval None
 */
void _Error_Handler(char* file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while (1) {}
  /* USER CODE END Error_Handler_Debug */
}
