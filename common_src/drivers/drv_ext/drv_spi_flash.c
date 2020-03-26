#include "drv_spi_flash.h"
#include "f0_bsp_spi.h"


// 写入时如果扇区需要擦除，则先将扇区读出并放入该缓存；
// 更新要写入的数据后，再写入原扇区；
uint8_t puc_temp_buf[W25QXX_SECTOR_SIZE];

static void prvDrvSPIFlashWriteEnable(void);
static void prv_drv_w25q_wait_for_write_end(void);

void v_drv_w25q_init(void)
{
  LL_GPIO_InitTypeDef  GPIO_InitStruct;

  LL_AHB1_GRP1_EnableClock(DRV_W25_CS_PORT_CLK);

  GPIO_InitStruct.Speed         = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Mode          = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.OutputType    = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull          = LL_GPIO_PULL_UP;
  GPIO_InitStruct.Pin           = DRV_W25_CS_PIN;
  LL_GPIO_Init(DRV_W25_CS_PORT, &GPIO_InitStruct);

  LOG_OUT_INF("JedecID: %x", ulDrvSPIFlashGetJedecID());
}

//sector = [0:1023]
void v_drv_w25q_sector_erase(uint32_t ul_sector_addr)
{
  prvDrvSPIFlashWriteEnable();

  ul_sector_addr = W25QXX_SECTOR_SIZE * ul_sector_addr;

  W25Q_CS(0);
  bsp_spi_exchange_data(W25Q_SPI, W25X_SECTOR_ERASE);
  bsp_spi_exchange_data(W25Q_SPI, (ul_sector_addr & 0xff0000) >> 16);
  bsp_spi_exchange_data(W25Q_SPI, (ul_sector_addr & 0xff00) >> 8);
  bsp_spi_exchange_data(W25Q_SPI, ul_sector_addr & 0xff);
  W25Q_CS(1);

  prv_drv_w25q_wait_for_write_end();
}

void v_drv_w25q_chip_erase(void)
{
  prvDrvSPIFlashWriteEnable();

  W25Q_CS(0);
  bsp_spi_exchange_data(W25Q_SPI, W25X_CHIP_ERASE);
  W25Q_CS(1);

  prv_drv_w25q_wait_for_write_end();
}

void v_drv_w25q_read(uint8_t* puc_des, uint32_t ul_addr, uint16_t us_len)
{
  W25Q_CS(0);
  bsp_spi_exchange_data(W25Q_SPI, W25X_RD_DATA);
  bsp_spi_exchange_data(W25Q_SPI, (ul_addr & 0xff0000) >> 16);
  bsp_spi_exchange_data(W25Q_SPI, (ul_addr & 0xff00) >> 8);
  bsp_spi_exchange_data(W25Q_SPI, ul_addr & 0xff);
  while(us_len--) {
    *puc_des++ = bsp_spi_exchange_data(W25Q_SPI, 0xff);
  }
  W25Q_CS(1);
}

void v_drv_w25q_page_write_no_check(uint8_t* puc_srs, uint32_t ul_addr, uint16_t us_len)
{
  prvDrvSPIFlashWriteEnable();
  prv_drv_w25q_wait_for_write_end();

  W25Q_CS(0);
  bsp_spi_exchange_data(W25Q_SPI, W25X_PAGE_WR);
  bsp_spi_exchange_data(W25Q_SPI, (ul_addr & 0xff0000) >> 16);
  bsp_spi_exchange_data(W25Q_SPI, (ul_addr & 0xff00) >> 8);
  bsp_spi_exchange_data(W25Q_SPI, ul_addr & 0xff);
  while(us_len--) {
    bsp_spi_exchange_data(W25Q_SPI, *puc_srs++);
  }
  W25Q_CS(1);

  prv_drv_w25q_wait_for_write_end();
}

void v_drv_w25q_brust_write_no_check(uint8_t* puc_srs, uint32_t ul_addr, uint16_t us_len)
{
  uint16_t us_page_remain = W25QXX_PAGE_SIZE - ul_addr % W25QXX_PAGE_SIZE;

  if(us_len <= us_page_remain)
    us_page_remain = us_len;

  while(1) {
    v_drv_w25q_page_write_no_check(puc_srs, ul_addr, us_page_remain);
    if(us_page_remain == us_len)
      break;
    else {
      puc_srs += us_page_remain;
      ul_addr += us_page_remain;

      us_len -= us_page_remain;
      if(us_len > W25QXX_PAGE_SIZE)
        us_page_remain = W25QXX_PAGE_SIZE;
      else
        us_page_remain = us_len;
    }
  }
}

//write data into specified address, with check
void v_drv_w25q_brust_write_check(uint8_t* puc_srs, uint32_t ul_addr, uint16_t us_len)
{
  uint16_t us_sec_position = ul_addr / W25QXX_SECTOR_SIZE;
  uint16_t us_sec_offset = ul_addr % W25QXX_SECTOR_SIZE;
  uint16_t us_sec_remain = W25QXX_SECTOR_SIZE - us_sec_offset;

  if(us_len <= us_sec_remain) {
    us_sec_remain = us_len;
  }

  uint16_t i = 0;

  while(1) {
    v_drv_w25q_read(puc_temp_buf,
                    us_sec_position * W25QXX_SECTOR_SIZE,
                    W25QXX_SECTOR_SIZE);

    for(i = 0; i < us_sec_remain; i++) {
      if(puc_temp_buf[us_sec_offset + i] != 0xff)
        break;
    }

    if(i < us_sec_remain) {
      v_drv_w25q_sector_erase(us_sec_position);

      for(i = 0; i < us_sec_remain; i++) {
        puc_temp_buf[i + us_sec_offset] = puc_srs[i];
      }

      v_drv_w25q_brust_write_no_check(puc_temp_buf,
                                      us_sec_position * W25QXX_SECTOR_SIZE,
                                      W25QXX_SECTOR_SIZE);
    } else {
      v_drv_w25q_brust_write_no_check(puc_srs, ul_addr, us_sec_remain);
    }

    if(us_len == us_sec_remain) {
      break;
    } else {
      us_sec_position += 1;
      us_sec_offset = 0;

      puc_srs += us_sec_remain;
      ul_addr += us_sec_remain;
      us_len -= us_sec_remain;

      if(us_len > W25QXX_SECTOR_SIZE) {
        us_sec_remain = W25QXX_SECTOR_SIZE;
      } else {
        us_sec_remain = us_len;
      }
    }
  }
}

uint32_t ulDrvSPIFlashGetJedecID(void)
{
  uint32_t temp = 0;

  //send write enable instruction
  prvDrvSPIFlashWriteEnable();

  //select the flash: chip select low
  W25Q_CS(0);
  bsp_spi_exchange_data(W25Q_SPI, 0x9f);
  temp |= bsp_spi_exchange_data(W25Q_SPI, 0xff) << 16;
  temp |= bsp_spi_exchange_data(W25Q_SPI, 0xff) << 8;
  temp |= bsp_spi_exchange_data(W25Q_SPI, 0xff);

  W25Q_CS(1);

  return temp;
}

static void prvDrvSPIFlashWriteEnable(void)
{
  //select the spi flash
  W25Q_CS(0);

  //send out write enable instruction
  bsp_spi_exchange_data(W25Q_SPI, W25X_WR_ENABLE);

  //deselect the spi flash
  W25Q_CS(1);
}

static void prv_drv_w25q_wait_for_write_end(void)
{
  uint8_t flash_status = 0;

  //select the spi flash
  W25Q_CS(0);

  //send out get status instruction
  bsp_spi_exchange_data(W25Q_SPI, W25X_RD_STATUS);

  //Loop as long as the memory is busy with a write cycle
  do {
    // Send a dummy byte(0xff) to generate the clock needed by the FLASH
    //and put the value of the status register in FLASH_Status variable
    flash_status = bsp_spi_exchange_data(W25Q_SPI, 0xff);
  } while ((flash_status & W25X_BUSY_MASK) == SET); //Write in progress

  //deselect the spi flash
  W25Q_CS(1);
}












