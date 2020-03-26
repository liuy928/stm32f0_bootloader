
// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "system_platform_config.h"

#include "system_poll_timer.h"
#include "system_task.h"

#include "ymodem.h"

#include "f0_bsp_usart.h"
#include "f0_bsp_led.h"
#include "f0_bsp_spi.h"
#include "f0_bsp_flash.h"

#include "drv_spi_flash.h"

#if SYS_USING_LOG_OUTPUT
#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

//PUTCHAR_PROTOTYPE
//{
//  /* Place your implementation of fputc here */
//  /* e.g. write a character to the USART */
//  USART_SendData(SYS_LOG_OUTPUT_UART, (uint8_t) ch);
//  /* Loop until the end of transmission */
//  while (USART_GetFlagStatus(SYS_LOG_OUTPUT_UART, USART_FLAG_TC) == RESET);
//  return ch;
//}
#endif // #if SYS_USING_LOG_OUTPUT

#if SYSTEM_USING_USART1
LL_USART_InitTypeDef drv_usart1_init_paras = {
  .BaudRate  = BSP_USART_BAUD_115200,
  .DataWidth = LL_USART_DATAWIDTH_8B,
  .StopBits  = LL_USART_STOPBITS_1,
  .Parity    = LL_USART_PARITY_NONE,
  .TransferDirection = LL_USART_DIRECTION_TX_RX,
  .HardwareFlowControl = LL_USART_HWCONTROL_NONE,
  .OverSampling = LL_USART_OVERSAMPLING_16,
};
#endif // #if SYSTEM_USING_USART1

#if SYSTEM_USING_USART2
LL_USART_InitTypeDef drv_usart2_init_paras = {
  .BaudRate  = BSP_USART_BAUD_115200,
  .DataWidth = LL_USART_DATAWIDTH_8B,
  .StopBits  = LL_USART_STOPBITS_1,
  .Parity    = LL_USART_PARITY_NONE,
  .TransferDirection = LL_USART_DIRECTION_TX_RX,
  .HardwareFlowControl = LL_USART_HWCONTROL_NONE,
  .OverSampling = LL_USART_OVERSAMPLING_16,
};
#endif // #if SYSTEM_USING_USART2

#if SYSTEM_USING_USART3
LL_USART_InitTypeDef drv_usart3_init_paras = {
  .BaudRate  = BSP_USART_BAUD_115200,
  .DataWidth = LL_USART_DATAWIDTH_8B,
  .StopBits  = LL_USART_STOPBITS_1,
  .Parity    = LL_USART_PARITY_NONE,
  .TransferDirection = LL_USART_DIRECTION_TX_RX,
  .HardwareFlowControl = LL_USART_HWCONTROL_NONE,
  .OverSampling = LL_USART_OVERSAMPLING_16,
};
#endif // #if SYSTEM_USING_USART3


typedef void (*pFunction)(void);
pFunction Jump_To_Application;
uint32_t JumpAddress;

/*******************************************************************************
  Function:
    void system_initialize(void)

  Summary:
    Initializes the board, services, drivers, application and other modules.

 */
void vSystemInit(void)
{
  /* Initialize Drivers */
  vSystemClockInit();
  vSystemSystickInit();
  vBspLedInit();
  bsp_spi_config(SPI2);
  v_drv_w25q_init();

#if SYSTEM_USING_USART2
  vBspUsartInit(&bspUsart2,
                &drv_usart2_init_paras,
                kPrioL3,
                BSP_USART_OPEN_INT_IDLE
                  | BSP_USART_OPEN_DMA_RX_BOOT
                  | BSP_USART_OPEN_POLLING);
#endif // #if SYSTEM_USING_USART2

  /* Initialize System Services */

  /* Initialize Middleware */

#if SYSTEM_USING_RTOS
  /* Initialize the Application */
  app_initialize();
#endif // #if SYSTEM_USING_RTOS
}

// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
uint8_t _pus_buffer_move[FLASH_PAGE_SIZE];
int main(void)
{
  /* Initialize all modules, including applic ation(s). */
  vSystemInit();

  // 上电后如果检测到按键按下超过2s，进入IAP模式；
  if(!LL_GPIO_IsInputPinSet(DRV_BTN_PORT, DRV_BTN_PIN)) {
    vSystemDelayXms(2000);
    if(!LL_GPIO_IsInputPinSet(DRV_BTN_PORT, DRV_BTN_PIN)) {
      goto label_running_iap;
    }
  }

  uint8_t _uc_flag = 0xff;

  // 固件备份区检测；
  v_drv_w25q_read(&_uc_flag, FLASH_FLAG_BACKUP_OK_ADDR, 1);
  if(_uc_flag != FLASH_FLAG_BACKUP_OK_VALUE) {
    // 备份区不存在有效固件，则直接跳转到运行区是否包含有效固件的检测过程；
    goto label_check_running;
  }

  uint32_t _ul_bin_size = 0;

  v_drv_w25q_read((uint8_t *)&_ul_bin_size, FLASH_IAP_APP_SIZE_ADDR, 4);
  if(_ul_bin_size >= APP_MAX_SIZE) {
    // 写入备份区无效的标志；
    _uc_flag = 0xff;
    v_drv_w25q_brust_write_check(&_uc_flag, FLASH_FLAG_BACKUP_OK_ADDR, 1);
    // 备份区固件过大，则直接跳转到运行区是否包含有效固件的检测过程；
    goto label_check_running;
  }

  // 统计内部flash容纳固件需要的扇区数；
  uint8_t _uc_internal_sector_num = _ul_bin_size / FLASH_PAGE_SIZE;
  if((_uc_internal_sector_num % FLASH_PAGE_SIZE) != 0) {
    _uc_internal_sector_num += 1;
  }

  // 准备开始转移固件，写入运行区无效的标志；
  _uc_flag = 0xff;
  v_drv_w25q_brust_write_check(&_uc_flag, FLASH_FLAG_RUNNIGN_OK_ADDR, 1);

  // 擦除运行区扇区；
  vBspFlashEraseNPage(FLASH_APP_RUNING_ADDRESS, _uc_internal_sector_num);

  // 转移固件；
  uint32_t _ul_bin_moved_size = 0;
  uint32_t _ul_external_read_addr = STORAGE_IAP_BACKUP_START_ADDR;
  uint32_t _ul_internal_write_addr = FLASH_APP_RUNING_ADDRESS;
  while(_ul_bin_moved_size < _ul_bin_size) {
    vBspLedToggle(kLed3);

    v_drv_w25q_read(_pus_buffer_move, _ul_external_read_addr, FLASH_PAGE_SIZE);
    bsp_flash_write_n_hword_nocheck(_ul_internal_write_addr,
                                    _pus_buffer_move,
                                    FLASH_PAGE_SIZE / 2);

    _ul_bin_moved_size += FLASH_PAGE_SIZE;
    _ul_external_read_addr += FLASH_PAGE_SIZE;
    _ul_internal_write_addr += FLASH_PAGE_SIZE;
  }

  // 是否转移完成；
  if(_ul_bin_moved_size >= _ul_bin_size) {
    // 转移固件完成，写入运行区有效的标志；
    _uc_flag = FLASH_FLAG_RUNNIGN_OK_VALUE;
    v_drv_w25q_brust_write_check(&_uc_flag, FLASH_FLAG_RUNNIGN_OK_ADDR, 1);

    // 写入备份区无效的标志；
    _uc_flag = 0xff;
    v_drv_w25q_brust_write_check(&_uc_flag, FLASH_FLAG_BACKUP_OK_ADDR, 1);
  }

label_check_running:
  v_drv_w25q_read(&_uc_flag, FLASH_FLAG_RUNNIGN_OK_ADDR, 1);
  if(_uc_flag == FLASH_FLAG_RUNNIGN_OK_VALUE) {
    // 依据实际的RAM空间大小修改该判断条件；
    if (((*(__IO uint32_t *)FLASH_APP_RUNING_ADDRESS) & 0x2FFE0000) == 0x20000000) {
      __set_PRIMASK(1);
      LL_DMA_DeInit(bspUsart2.px_rx_dma, bspUsart2.ul_rx_dma_channel);

      //跳转至用户代码
      Jump_To_Application =
        (pFunction)(*(__IO uint32_t *)(FLASH_APP_RUNING_ADDRESS + 4));
      //初始化用户程序的堆栈指针
      __set_MSP(*(__IO uint32_t *)FLASH_APP_RUNING_ADDRESS);
       Jump_To_Application();
    }
  }

label_running_iap:

  while (1) {
    // 周期性扫描等待ymoderm接收到有效固件；
    if (ymodem_receive(&_ul_bin_size) == ERR_OK) {
      // 保存bin文件大小
      v_drv_w25q_brust_write_check((uint8_t *)&_ul_bin_size,
                                   FLASH_IAP_APP_SIZE_ADDR,
                                   4);
      // 写入备份区有效的标志；
      _uc_flag = FLASH_FLAG_BACKUP_OK_VALUE;
      v_drv_w25q_brust_write_check(&_uc_flag, FLASH_FLAG_BACKUP_OK_ADDR, 1);

      //重启系统；
      NVIC_SystemReset();
    }
  }
}
