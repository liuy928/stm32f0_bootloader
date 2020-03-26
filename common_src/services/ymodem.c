/**
  ******************************************************************************
  * @file    IAP_Main/Src/ymodem.c
  * @author  MCD Application Team
  * @version 1.0.0
  * @date    8-April-2015
  * @brief   This file provides all the software functions related to the ymodem
  *          protocol.
  ******************************************************************************
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/** @addtogroup STM32F1xx_IAP
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "ymodem.h"
#include "string.h"

#include "f0_bsp_led.h"
#include "f0_bsp_usart.h"
#include "drv_spi_flash.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* @note ATTENTION - please keep this variable 32bit alligned */
uint16_t us_packet_len = 0;
uint8_t aPacketData[PACKET_1K_SIZE + PACKET_DATA_INDEX + PACKET_TRAILER_SIZE];
uint8_t aFileName[FILE_NAME_LENGTH];

/* Private functions ---------------------------------------------------------*/

static YMODEM_STATUS_TYPE _ymodem_read_buf_wait(xBSPUsartHandle_t* usart_index,
                                                uint16_t us_read_offset,
                                                uint8_t *pdata,
                                                uint16_t size,
                                                uint16_t timeout)
{
  uint16_t i = 0;

  // 等待串口DMA收到数据；
  while (us_packet_len == 0) {
    i++;
    vSystemDelayXms(1);

    if (i > timeout) {
      vBspLedToggle(kLed3);

      return YMODEM_TIMEOUT;
    }
  }

  // 从buffer读取数据；
  memcpy(pdata, usart_index->px_rx_buff->puc_buffer + us_read_offset, size);

  return YMODEM_OK;
}

void ymodem_int_idle_callback(void)
{
  // 关闭DMA
  LL_DMA_DisableChannel(bspUsart2.px_rx_dma, bspUsart2.ul_rx_dma_channel);

  //获取接受长度
  us_packet_len = bspUsart2.px_rx_buff->us_size
                  - LL_DMA_GetDataLength(bspUsart2.px_rx_dma, bspUsart2.ul_rx_dma_channel);

  //重启DMA
  vBspUsartDmaRxRestart(&bspUsart2, (&bspUsart2)->px_rx_buff->us_size);
}

/**
  * @brief  Update CRC16 for input byte
  * @param  crc_in input value
  * @param  input byte
  * @retval None
  */
static uint16_t UpdateCRC16(uint16_t crc_in, uint8_t byte)
{
  uint32_t crc = crc_in;
  uint32_t in = byte | 0x100;

  do {
    crc <<= 1;
    in <<= 1;
    if (in & 0x100) ++crc;
    if (crc & 0x10000) crc ^= 0x1021;
  }

  while (!(in & 0x10000));

  return crc & 0xffffu;
}

/**
  * @brief  Cal CRC16 for YModem Packet
  * @param  data
  * @param  length
  * @retval None
  */
uint16_t Cal_CRC16(const uint8_t* p_data, uint32_t size)
{
  uint32_t crc = 0;
  const uint8_t *dataEnd = p_data + size;

  while (p_data < dataEnd) crc = UpdateCRC16(crc, *p_data++);

  crc = UpdateCRC16(crc, 0);
  crc = UpdateCRC16(crc, 0);

  return crc & 0xffffu;
}

/**
  * @brief  Calculate Check sum for YModem Packet
  * @param  p_data Pointer to input data
  * @param  size length of input data
  * @retval uint8_t checksum value
  */
uint8_t CalcChecksum(const uint8_t *p_data, uint32_t size)
{
  uint32_t sum = 0;
  const uint8_t *p_data_end = p_data + size;

  while (p_data < p_data_end) {
    sum += *p_data++;
  }

  return (sum & 0xffu);
}

/**
  * @brief  Receive a packet from sender
  * @param  data
  * @param  length
  *     0: end of transmission
  *     2: abort by sender
  *    >0: packet length
  * @param  timeout
  * @retval HAL_OK: normally return
  *         HAL_BUSY: abort by user
  */
static YMODEM_STATUS_TYPE _ymodem_receive_packet(uint8_t *p_data,
                                                 uint32_t *p_length,
                                                 uint32_t timeout)
{
  uint32_t crc;
  uint32_t packet_size = 0;
  YMODEM_STATUS_TYPE status;
  uint8_t char1;

  *p_length = 0;

  if (_ymodem_read_buf_wait(&bspUsart2, 0, &char1, 1, timeout) != YMODEM_OK) {
    return YMODEM_ERROR;
  }

  switch (char1) {
    case SOH:
      packet_size = PACKET_SIZE;

      break;

    case STX:
      packet_size = PACKET_1K_SIZE;

      break;

    case EOT:
      return YMODEM_OK;

      break;

    case CA:
      if ((_ymodem_read_buf_wait(&bspUsart2, 1, &char1, 1, timeout) == YMODEM_OK) &&
          (char1 == CA)) {
        packet_size = 2;

        return YMODEM_OK;
      } else {
        return YMODEM_ERROR;
      }

      break;

    case ABORT1:
    case ABORT2:
      return YMODEM_BUSY;

      break;

    default:
      return YMODEM_ERROR;

      break;
  }

  *p_length = packet_size;

  // 保存第一个字节；
  *p_data = char1;
  // 接收剩余的数据；
  if (packet_size >= PACKET_SIZE) {
    status = _ymodem_read_buf_wait(&bspUsart2,
                                   PACKET_NUMBER_INDEX,
                                   &p_data[PACKET_NUMBER_INDEX],
                                   packet_size + PACKET_OVERHEAD_SIZE,
                                   timeout);

    /* Simple packet sanity check */
    if (status == YMODEM_OK) {
      if (p_data[PACKET_NUMBER_INDEX] != ((p_data[PACKET_CNUMBER_INDEX]) ^ NEGATIVE_BYTE)) {
        packet_size = 0;

        return YMODEM_ERROR;
      } else {
        /* Check packet CRC */
        crc = p_data[packet_size + PACKET_DATA_INDEX] << 8;
        crc += p_data[packet_size + PACKET_DATA_INDEX + 1];
        if (Cal_CRC16(&p_data[PACKET_DATA_INDEX], packet_size) != crc) {
          packet_size = 0;

          return YMODEM_ERROR;
        }
      }
    } else {
      packet_size = 0;

      return YMODEM_ERROR;
    }
  }

  return YMODEM_OK;
}

/**
  * @brief  Convert a string to an integer
  * @param  p_inputstr: The string to be converted
  * @param  p_intnum: The integer value
  * @retval 1: Correct
  *         0: Error
  */
uint32_t Str2Int(uint8_t *p_inputstr, uint32_t *p_intnum)
{
  uint32_t i = 0, res = 0;
  uint32_t val = 0;

  if ((p_inputstr[0] == '0') &&
      ((p_inputstr[1] == 'x') || (p_inputstr[1] == 'X'))) {
    i = 2;
    while ((i < 11) && (p_inputstr[i] != '\0')) {
      if (ISVALIDHEX(p_inputstr[i])) {
        val = (val << 4) + CONVERTHEX(p_inputstr[i]);
      } else {
        /* Return 0, Invalid input */
        res = 0;
        break;
      }
      i++;
    }

    /* valid result */
    if (p_inputstr[i] == '\0') {
      *p_intnum = val;
      res = 1;
    }
  } else /* max 10-digit decimal input */
  {
    while ((i < 11) && (res != 1)) {
      if (p_inputstr[i] == '\0') {
        *p_intnum = val;
        /* return 1 */
        res = 1;
      } else if (((p_inputstr[i] == 'k') || (p_inputstr[i] == 'K')) &&
                 (i > 0)) {
        val = val << 10;
        *p_intnum = val;
        res = 1;
      } else if (((p_inputstr[i] == 'm') || (p_inputstr[i] == 'M')) &&
                 (i > 0)) {
        val = val << 20;
        *p_intnum = val;
        res = 1;
      } else if (ISVALIDDEC(p_inputstr[i])) {
        val = val * 10 + CONVERTDEC(p_inputstr[i]);
      } else {
        /* return 0, Invalid input */
        res = 0;
        break;
      }
      i++;
    }
  }

  return res;
}

/* Public functions ---------------------------------------------------------*/
/**
  * @brief  Receive a file using the ymodem protocol with CRC16.
  * @param  p_size The size of the file.
  * @retval COM_StatusTypeDef result of reception/programming
  */
YMODEM_ERROR_TYPE ymodem_receive(uint32_t *p_size)
{
  uint32_t i, packet_length;

  uint32_t ramsource, filesize;
  uint8_t *file_ptr;

  uint8_t file_size[FILE_SIZE_LENGTH], tmp;

  /* Initialize flashdestination variable */
  uint32_t _ul_dest_addr = STORAGE_IAP_BACKUP_START_ADDR;

  uint8_t packets_received;
  uint32_t session_done = 0, errors = 0, session_begin = 0, file_done = 0;
  for (uint32_t session_done = 0, errors = 0, session_begin = 0;;) {
    for (packets_received = 0, file_done = 0;;) {
      YMODEM_STATUS_TYPE x_res =
        _ymodem_receive_packet(aPacketData, &packet_length, DOWNLOAD_TIMEOUT);
      us_packet_len = 0;
      switch (x_res) {
      case YMODEM_OK:
        errors = 0;

        switch (packet_length) {
          case 2:
            /* Abort by sender */
            vBspUsartByteWrite(&bspUsart2, ACK);

            return ERR_ABORT;

            break;

          case 0:
            /* End of transmission */
            vBspUsartByteWrite(&bspUsart2, ACK);
            vBspUsartByteWrite(&bspUsart2, CRC16);
            file_done = 1;

            break;

          default:
            /* Normal packet */
            if (aPacketData[PACKET_NUMBER_INDEX] != packets_received) {
              vBspUsartByteWrite(&bspUsart2, NAK);
            } else {
              // 0号数据包；
              if (packets_received == 0) {
                vBspLedToggle(kLed3);

                // File name packet
                if (aPacketData[PACKET_DATA_INDEX] != 0) {
                  // File name extraction
                  i = 0;
                  file_ptr = aPacketData + PACKET_DATA_INDEX;
                  while ((*file_ptr != 0) && (i < FILE_NAME_LENGTH)) {
                    aFileName[i++] = *file_ptr++;
                  }
                  /* File size extraction */
                  aFileName[i++] = '\0';
                  i = 0;
                  file_ptr++;
                  while ((*file_ptr != ' ') && (i < FILE_SIZE_LENGTH - 1)) {
                    file_size[i++] = *file_ptr++;
                  }
                  file_size[i++] = '\0';
                  Str2Int(file_size, &filesize);

                  /* Test the size of the image to be sent */
                  /* Image size is greater than Flash size */
                  if (filesize > (APP_MAX_SIZE + 1)) {
                    /* End session */
                    tmp = CA;
                    usBspUsartWrite(&bspUsart2, &tmp, 1);
                    usBspUsartWrite(&bspUsart2, &tmp, 1);

                    return ERR_LIMIT;
                  }
                  *p_size = filesize;

                  vBspUsartByteWrite(&bspUsart2, ACK);
                  vBspUsartByteWrite(&bspUsart2, CRC16);
                // File header packet is empty, end session
                } else {
                  vBspUsartByteWrite(&bspUsart2, ACK);
                  file_done = 1;
                  session_done = 1;

                  break;
                }
              // Data packet
              } else {
                vBspLedToggle(kLed3);

                ramsource = (uint32_t)&aPacketData[PACKET_DATA_INDEX];

                /* Write received data in Flash */
                v_drv_w25q_brust_write_check((uint8_t *)ramsource,
                                             _ul_dest_addr,
                                             packet_length);
                _ul_dest_addr += packet_length;

                vBspUsartByteWrite(&bspUsart2, ACK);

                /* An error occurred while writing to Flash memory */
                //} else {
                  /* End session */
                  // vBspUsartByteWrite(&bspUsart2, CA);
                  // vBspUsartByteWrite(&bspUsart2, CA);

                  // return ERR_DATA;
               // }
              }

              packets_received++;
              session_begin = 1;
            }
            break;
        }

        break;

      case YMODEM_BUSY: /* Abort actually */
        vBspUsartByteWrite(&bspUsart2, CA);
        vBspUsartByteWrite(&bspUsart2, CA);

        return ERR_ABORT;

        break;

      default:
        if (session_begin > 0) {
          errors++;
        }
        if (errors > MAX_ERRORS) {
          /* Abort communication */
          vBspUsartByteWrite(&bspUsart2, CA);
          vBspUsartByteWrite(&bspUsart2, CA);

          return ERR_ABORT;
        }

        vBspUsartByteWrite(&bspUsart2, CRC16); /* Ask for a packet */

        break;
      }

      if (file_done != 0) {
        break;
      }
    }

    if (session_done != 0) {
      break;
    }
  }

  return ERR_OK;
}


/**
  * @}
  */

/*******************(C)COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
