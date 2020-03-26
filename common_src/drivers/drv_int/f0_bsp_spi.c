#include "f0_bsp_spi.h"

static void _bsp_spi_gpio_config(SPI_TypeDef* SPIx)
{
  LL_GPIO_InitTypeDef GPIO_InitStruct;

  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;

  if(SPIx == SPI1) {
    LL_AHB1_GRP1_EnableClock(DRV_SPI1_PORT_CLK);
	  LL_APB1_GRP2_EnableClock(DRV_SPI1_CLK);

    GPIO_InitStruct.Alternate   = LL_GPIO_AF_0;
    GPIO_InitStruct.Pin         = DRV_SPI1_MOSI_PIN
                                  | DRV_SPI1_MISO_PIN
                                  | DRV_SPI1_SCLK_PIN;
    LL_GPIO_Init(DRV_SPI1_PORT, &GPIO_InitStruct);
  } else if(SPIx == SPI2) {
    LL_AHB1_GRP1_EnableClock(DRV_SPI2_PORT_CLK);
	  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);

    GPIO_InitStruct.Alternate   = LL_GPIO_AF_0;
    GPIO_InitStruct.Pin         = DRV_SPI2_MOSI_PIN
                                  | DRV_SPI2_MISO_PIN | DRV_SPI2_SCLK_PIN;
    LL_GPIO_Init(DRV_SPI2_PORT, &GPIO_InitStruct);
//    GPIO_InitStruct.Alternate   = LL_GPIO_AF_5;
//    GPIO_InitStruct.Pin         = DRV_SPI2_SCLK_PIN;
//    LL_GPIO_Init(DRV_SPI2_PORT, &GPIO_InitStruct);
  }
}

void bsp_spi_config(SPI_TypeDef* SPIx)
{
  LL_SPI_InitTypeDef SPI_InitStruct;

  _bsp_spi_gpio_config(SPIx);

  SPI_InitStruct.TransferDirection        = LL_SPI_FULL_DUPLEX;
  SPI_InitStruct.Mode                     = LL_SPI_MODE_MASTER;
  SPI_InitStruct.DataWidth                = LL_SPI_DATAWIDTH_8BIT;
  SPI_InitStruct.ClockPolarity            = LL_SPI_POLARITY_HIGH;
  SPI_InitStruct.ClockPhase               = LL_SPI_PHASE_2EDGE;
  SPI_InitStruct.NSS                      = LL_SPI_NSS_SOFT;
  SPI_InitStruct.BaudRate                 = LL_SPI_BAUDRATEPRESCALER_DIV8;
  SPI_InitStruct.BitOrder                 = LL_SPI_MSB_FIRST;
  SPI_InitStruct.CRCCalculation           = LL_SPI_CRCCALCULATION_DISABLE;
  SPI_InitStruct.CRCPoly                  = 7;
  LL_SPI_Init(SPIx, &SPI_InitStruct);

  // LL_SPI_SetStandard(SPI2, LL_SPI_PROTOCOL_MOTOROLA);
  LL_SPI_SetRxFIFOThreshold(SPIx, LL_SPI_RX_FIFO_TH_QUARTER);

  LL_SPI_Enable(SPIx);
}

uint8_t bsp_spi_exchange_data(SPI_TypeDef* SPIx, uint8_t dat)
{
  while( LL_SPI_IsActiveFlag_TXE(SPIx)  == RESET);
  LL_SPI_TransmitData8(SPIx, dat);
  while(LL_SPI_IsActiveFlag_RXNE(SPIx) == RESET);
  return LL_SPI_ReceiveData8(SPIx);
}
