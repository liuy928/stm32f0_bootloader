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

#include "f0_bsp_flash.h"
#include "f0_bsp_led.h"
#include "f0_bsp_usart.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* @note ATTENTION - please keep this variable 32bit alligned */

typedef struct {
  uint32_t ul_file_size;
  char pc_file_name[FILE_NAME_LENGTH];

  uint8_t puc_packet_buffer[PACKET_1K_SIZE + PACKET_DATA_INDEX + PACKET_TRAILER_SIZE];
} xYmodemManage_t;

xYmodemManage_t x_ymodem_ins = {
  .ul_file_size = 0,
};

// 用于存储将要写入到内部flash的数据；
uint8_t puc_flash_buffer[PACKET_1K_SIZE];

/* Private functions ---------------------------------------------------------*/

static uint8_t puc_ymodem_itoa(uint32_t ul_val, uint8_t* puc_buf)
{
  uint8_t ci = 0;

  if (ul_val == 0) {
    puc_buf[ci++] = '0';
  } else {
    uint8_t s[11];
    int8_t i = sizeof(s) - 1;

    /* z-terminate string */
    s[i] = '\0';

    while ((ul_val > 0) && (i > 0)) {
      /* write decimal char */
      s[--i] = (ul_val % 10) + '0';
      ul_val /= 10;
    }

    uint8_t* sp = &s[i];
    while (*sp) {
      puc_buf[ci++] = *sp++;
    }
  }
  /* z-term */
  puc_buf[ci] = '\0';

  return ci;
}

static xYmodemStatus_t prx_ymodem_read_buf(
    xBspUsartInstant_t* px_usart_index, uint8_t* puc_data, uint16_t us_len, uint16_t us_timeout)
{
  uint16_t i = 0;

  // 等待串口DMA收到数据；
  while (i <= us_timeout) {
    i++;
    vSystemDelayXms(10);

    // 更新索引；
    us_bsp_usart_dma_rec_fifo_real_in_get(px_usart_index);

    if (us_fifo_get_valid_length(YMODEM_UART_PORT->px_rx_buff) >= us_len) {
      if (us_bsp_usart_read(px_usart_index, puc_data, us_len) == us_len) {
        return YMODEM_OK;
      }
    }
  }

  return YMODEM_TIMEOUT;
}

/* Ymodem校验计算部分 ----------------------------------------------------------*/
/**
 * @brief  Update CRC16 for input byte
 * @param  crc_in input value
 * @param  input byte
 * @retval None
 */
static uint16_t prus_ymodem_update_crc16(uint16_t us_crc_data, uint8_t uc_byte)
{
  uint32_t ul_crc = us_crc_data;
  uint32_t ul_in = uc_byte | 0x100;

  do {
    ul_crc <<= 1;
    ul_in <<= 1;
    if (ul_in & 0x100) {
      ++ul_crc;
    }

    if (ul_crc & 0x10000) {
      ul_crc ^= 0x1021;
    }
  } while (!(ul_in & 0x10000));

  return ul_crc & 0xffffu;
}

/**
 * @brief  Cal CRC16 for YModem Packet
 * @param  data
 * @param  length
 * @retval None
 */
uint16_t us_cal_crc16(const uint8_t* puc_data, uint32_t ul_size)
{
  uint32_t crc = 0;
  const uint8_t* dataEnd = puc_data + ul_size;

  while (puc_data < dataEnd) {
    crc = prus_ymodem_update_crc16(crc, *puc_data++);
  }

  crc = prus_ymodem_update_crc16(crc, 0);
  crc = prus_ymodem_update_crc16(crc, 0);

  return crc & 0xffffu;
}

/* Ymodem接收部分 -------------------------------------------------------------*/

// @brief 接收一个Ymodem数据包；
// @param puc_data - 接收到的数据包存放的位置；
// @param pul_length - 接收到的数据长度；
//        0: end of transmission
//        2: abort by sender
//        大于0: packet length
// @param ul_timeout - 等待超时；
// @return HAL_OK: normally return
//         HAL_BUSY: abort by user
static xYmodemStatus_t prx_ymodem_receive_packet(
    uint8_t* puc_data, uint16_t* pus_length, uint32_t ul_timeout)
{
  uint8_t uc_byte;

  *pus_length = 0;

  // 从缓存读取一个字节的数据，判断数据包类型；
  if (prx_ymodem_read_buf(YMODEM_UART_PORT, &uc_byte, 1, ul_timeout) != YMODEM_OK) {
    return YMODEM_ERROR;
  }

  switch (uc_byte) {
  case SOH:
    *pus_length = PACKET_SIZE;

    break;

  case STX:
    *pus_length = PACKET_1K_SIZE;

    break;

  case EOT:
    return YMODEM_OK;

    break;

  case CA:
    if ((prx_ymodem_read_buf(YMODEM_UART_PORT, &uc_byte, 1, ul_timeout) == YMODEM_OK)
        && (uc_byte == CA)) {
      *pus_length = 2;

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

  // 收到正确长度的数据包；
  if (*pus_length >= PACKET_SIZE) {
    // 保存第一个字节；
    puc_data[0] = uc_byte;
    // 接收剩余的数据，并简要校验；
    if (prx_ymodem_read_buf(YMODEM_UART_PORT, &puc_data[PACKET_NUMBER_INDEX],
            *pus_length + PACKET_OVERHEAD_SIZE, ul_timeout)
        != YMODEM_OK) {
      *pus_length = 0;

      return YMODEM_ERROR;
    }

    if (puc_data[PACKET_NUMBER_INDEX] != ((puc_data[PACKET_CNUMBER_INDEX]) ^ NEGATIVE_BYTE)) {
      *pus_length = 0;

      return YMODEM_ERROR;
    }

    if (us_cal_crc16(&puc_data[PACKET_DATA_INDEX], *pus_length)
        != USER_MAKE_HALF_WORD(puc_data[*pus_length + PACKET_DATA_INDEX],
            puc_data[*pus_length + PACKET_DATA_INDEX + 1])) {
      *pus_length = 0;

      return YMODEM_ERROR;
    }
  }

  return YMODEM_OK;
}

/**
 * @brief  Receive a file using the ymodem protocol with CRC16.
 * @param  pul_size The size of the file.
 * @retval COM_StatusTypeDef result of reception/programming
 */
xYmodemError_t x_ymodem_receive(uint32_t* pul_size)
{
  // 定位bin文件辈份区域的首地址；
  uint32_t ul_dest_addr = FLASH_APP_BACKUP_ADDRESS;

  uint8_t uc_count_errors = 0;
  uint16_t us_packet_length = 0;
  xUserBool_t x_first_try = USR_TRUE;
  xUserBool_t x_session_done = USR_FALSE;

  do { // session process
    if (x_first_try == USR_TRUE) {
      v_bsp_usart_byte_write(YMODEM_UART_PORT, CRC16);
    }
    x_first_try = USR_FALSE;

    xUserBool_t x_file_done = USR_FALSE;
    uint8_t uc_packet_rxd = 0;

    do { // file process
      // 读取一个数据包；
      xYmodemStatus_t x_res = prx_ymodem_receive_packet(
          x_ymodem_ins.puc_packet_buffer, &us_packet_length, DOWNLOAD_TIMEOUT);
      // 判断数据包读取结果；
      switch (x_res) {
      case YMODEM_OK:
        uc_count_errors = 0;
        // 判断接受的数据长度；
        switch (us_packet_length) {
        case 2: // 发送端终止发送，发送响应信号；
          v_bsp_usart_byte_write(YMODEM_UART_PORT, ACK);

          return ERR_ABORT;

        case 0: // 收到的数据包是文件传输结束，EOT信号，给予响应；
          v_bsp_usart_byte_write(YMODEM_UART_PORT, ACK);

          // 文件接收完成；
          // TODO 可在此处增加文件是否正确的校验；
          x_file_done = USR_TRUE;

          // TODO 可在此处选择是否设置 x_first_try
          // 重新触发字符‘C’的发送，请求新的数据包；
          // 如果发送端没有新的文件要发送，那么会发送一个序号为0、内容全部为0的SOH数据包；
          x_first_try = USR_TRUE;

          // TODO 如果不需要请求新的文件，可在此处置位标志，结束当前传输：
          // x_session_done = USR_TRUE;

          break;

        default: // 正常的数据包；
                 ;
          uint8_t uc_packet_seq_nbr = x_ymodem_ins.puc_packet_buffer[PACKET_NUMBER_INDEX];
          // 包序号校验；
          if (uc_packet_seq_nbr != uc_packet_rxd) {
            v_bsp_usart_byte_write(YMODEM_UART_PORT, NAK);
          } else {
            // 解析0号数据包；包含了传输的文件名称、文件长度等信息；
            if (uc_packet_rxd == 0) {
              v_bsp_led_toggle(LED_IND);

              uint8_t i = 0;

              // 判断是否为空数据包；
              // 超级终端确实会在请求新的文件后，如果没有新的文件要发送会发送一个全‘\0’数据包；
              // 但是，SecureCRT发送的这个数据包并非全‘\0’数据包；
              for (i = PACKET_DATA_INDEX; i < (PACKET_DATA_INDEX + 4); i++) {
                if ((x_ymodem_ins.puc_packet_buffer[i] != '\0')
                    // 这个判断针对SecureCRT有效，
                    && (x_ymodem_ins.puc_packet_buffer[i] != '0')
                    && (x_ymodem_ins.puc_packet_buffer[i] != ' ')) {
                  break;
                }
              }

              // 检测到非空数据包；
              if (i < (PACKET_DATA_INDEX + 4)) {
                // File name extraction
                uint8_t* puc_file_name_ptr = x_ymodem_ins.puc_packet_buffer + PACKET_DATA_INDEX;
                i = 0;

                while ((*puc_file_name_ptr != 0) && (i < FILE_NAME_LENGTH)) {
                  x_ymodem_ins.pc_file_name[i++] = *puc_file_name_ptr++;
                }
                // 构成合法字符串；
                x_ymodem_ins.pc_file_name[i++] = '\0';

                // 跳过空格；
                puc_file_name_ptr++;

                // 获取文件字节数；
                i = 0;
                while ((*puc_file_name_ptr >= '0') && (*puc_file_name_ptr <= '9')) {
                  x_ymodem_ins.ul_file_size
                      = x_ymodem_ins.ul_file_size * 10 + (*puc_file_name_ptr - '0');
                  puc_file_name_ptr++;
                }

                // 判断文件是否过大；
                if (x_ymodem_ins.ul_file_size > (APP_MAX_SIZE + 1)) {
                  // 文件过长，终止传输；
                  v_bsp_usart_byte_write(YMODEM_UART_PORT, CA);
                  v_bsp_usart_byte_write(YMODEM_UART_PORT, CA);
                  vSystemDelayXms(1000);

                  return ERR_LIMIT;
                } else {
                  // 记录文件长度；
                  *pul_size = x_ymodem_ins.ul_file_size;
                  // 文件信息接收成功，发送应答；
                  v_bsp_usart_byte_write(YMODEM_UART_PORT, ACK);
                  // 发出字符‘C’，请求数据包；
                  v_bsp_usart_byte_write(YMODEM_UART_PORT, CRC16);
                }
              } else { // 全0数据包；
                v_bsp_usart_byte_write(YMODEM_UART_PORT, ACK);
                x_file_done = USR_TRUE;
                x_session_done = USR_TRUE;

                break;
              }
            } else { // Data packet
              v_bsp_led_toggle(LED_IND);

              // TODO
              // 测试发现，必须要先转移到一个全局数组后在写入内部Flash，否则直接写入的话会导致硬件错误；
              // 还没有分析具体原因；
              memcpy(puc_flash_buffer, &x_ymodem_ins.puc_packet_buffer[PACKET_DATA_INDEX],
                  us_packet_length);

              bsp_flash_write_n_hword_check(
                  ul_dest_addr, (void*)&puc_flash_buffer, sizeof(puc_flash_buffer) / 2);

              // 偏移flash写入的目标地址；
              ul_dest_addr += us_packet_length;

              // 发送应答，请求下一个数据包；
              v_bsp_usart_byte_write(YMODEM_UART_PORT, ACK);
            }

            uc_packet_rxd++;
          }
          break;
        }
        break;

      case YMODEM_BUSY:
        // 终止传输；
        v_bsp_usart_byte_write(YMODEM_UART_PORT, CA);
        v_bsp_usart_byte_write(YMODEM_UART_PORT, CA);

        return ERR_ABORT;

        break;

      default:
        if (uc_packet_rxd > 0) {
          uc_count_errors++;
        }

        if (uc_count_errors > MAX_ERRORS) {
          // 终止传输；
          v_bsp_usart_byte_write(YMODEM_UART_PORT, CA);
          v_bsp_usart_byte_write(YMODEM_UART_PORT, CA);

          return ERR_ABORT;
        }

        v_bsp_usart_byte_write(YMODEM_UART_PORT, CRC16);
        v_bsp_led_toggle(LED_IND);

        break;
      }
    } while (x_file_done == USR_FALSE);
  } while (x_session_done == USR_FALSE);

  return ERR_OK;
}

/* Ymodem发送部分 -------------------------------------------------------------*/

static void v_ymodem_send_packet(uint8_t* puc_tx_data, uint16_t us_block_nbr)
{
  if (puc_tx_data == NULL) {
    return;
  }

  uint16_t us_tx_packet_size = 0;
  uint8_t puc_packet[PACKET_1K_SIZE + PACKET_HEADER_SIZE + PACKET_TRAILER_SIZE];
  memset(puc_packet, '\0', sizeof(puc_packet));

  if (us_block_nbr == 0) {
    us_tx_packet_size = PACKET_SIZE;
  } else {
    us_tx_packet_size = PACKET_1K_SIZE;
  }

  // 计算CRC16；
  uint16_t us_crc16 = us_cal_crc16(puc_tx_data, us_tx_packet_size);

  // 构造数据包；
  puc_packet[0] = (us_block_nbr == 0) ? SOH : STX;
  // 数据包序号；
  puc_packet[1] = us_block_nbr & 0xff;
  puc_packet[2] = ~us_block_nbr & 0xff;
  // 数据载荷；
  memcpy((void*)(puc_packet + PACKET_HEADER_SIZE), puc_tx_data, us_tx_packet_size);
  // 校验数据；
  puc_packet[us_tx_packet_size + PACKET_HEADER_SIZE] = (us_crc16 >> 8) & 0xff;
  puc_packet[us_tx_packet_size + PACKET_HEADER_SIZE + 1] = us_crc16 & 0xff;

  // 发送数据包；
  us_bsp_usart_write(YMODEM_UART_PORT, puc_packet, sizeof(puc_packet));
}

static void v_ymodem_send_packet0(const char* pc_file_name, uint32_t ul_file_size)
{
  uint16_t us_pos = 0;
  uint8_t puc_packet[PACKET_SIZE];

  if (pc_file_name != NULL) {
    // 按照最大文件名称允许长度填充名称文本；
    while ((*pc_file_name != '\0') && (us_pos < FILE_NAME_LENGTH)) {
      puc_packet[us_pos++] = *pc_file_name++;
    }
    // 文件名填充结束，填充空格；
    puc_packet[us_pos++] = ' ';
    // 填充文件长度；
    us_pos += puc_ymodem_itoa(ul_file_size, &puc_packet[us_pos]);
  }

  while (us_pos < PACKET_SIZE) {
    puc_packet[us_pos] = '\0';
  }

  v_ymodem_send_packet(puc_packet, 0);
}

static void v_ymodem_send_packet_x(uint8_t* puc_tx_data, uint16_t us_tx_len, uint16_t us_timeout)
{
  uint8_t uc_byte = 0;
  uint16_t us_block_nbr = 1;
  uint8_t uc_retry = 0;

  while (us_tx_len > 0) {
    uint16_t us_send_size = (us_tx_len > PACKET_1K_SIZE) ? PACKET_1K_SIZE : us_tx_len;

    v_ymodem_send_packet(puc_tx_data, us_block_nbr);
    // 接收接收端的响应字符；
    prx_ymodem_read_buf(YMODEM_UART_PORT, &uc_byte, 1, us_timeout);
    switch (uc_byte) {
    case ACK: // 收到接收端的响应；
      puc_tx_data += us_send_size;
      us_tx_len -= us_send_size;
      us_block_nbr++;

      break;

    case CA: // 接收端终止接收；
      return;

    case NAK: // 重发数据包；
      v_ymodem_send_packet(puc_tx_data, us_block_nbr);
      if (uc_retry >= MAX_ERRORS) {
        v_bsp_usart_byte_write(YMODEM_UART_PORT, CA);
        v_bsp_usart_byte_write(YMODEM_UART_PORT, CA);

        return;
      }

      break;

    default:
      break;
    }
  }

  // 文件发送完成；
  uc_retry = 0;
  do {
    // 发送EOT信号，通知接收端文件发送完成；
    v_bsp_usart_byte_write(YMODEM_UART_PORT, EOT);
    // 接收接收端的响应字符；
    prx_ymodem_read_buf(YMODEM_UART_PORT, &uc_byte, 1, us_timeout);
  } while ((uc_byte != ACK) && (uc_retry++ < MAX_ERRORS));

  if (uc_byte == ACK) {
    prx_ymodem_read_buf(YMODEM_UART_PORT, &uc_byte, 1, us_timeout);
    // 如果接收端在接受完文件后，再次发送'C'，则发送端发送一个全0的0号数据包,
    // 通知接收端没有新的数据包要发送了；
    if (uc_byte == CRC16) {
      do {
        v_ymodem_send_packet0(0, 0);
        prx_ymodem_read_buf(YMODEM_UART_PORT, &uc_byte, 1, us_timeout);
      } while ((uc_byte != ACK) && (uc_retry++ < MAX_ERRORS));
    }
  }
}

uint32_t ul_ymodem_send(uint8_t* puc_tx_data, uint16_t us_len, const char* pc_file_name)
{
  uint8_t uc_byte = 0;
  uint8_t uc_retry = 0;

  // 接收接收端发过来的‘C’字符，确认接收端准备好了；
  do {
    prx_ymodem_read_buf(YMODEM_UART_PORT, &uc_byte, 1, DOWNLOAD_TIMEOUT);
  } while ((uc_byte != CRC16) && (uc_retry++ < MAX_ERRORS));

  xUserBool_t x_crc_nak = USR_TRUE, x_file_done = USR_FALSE;

  do {
    v_ymodem_send_packet0(pc_file_name, us_len);
    // 接收端成功解析到0号数据包后，会发送ACK和字符'C'请求数据包；
    prx_ymodem_read_buf(YMODEM_UART_PORT, &uc_byte, 1, DOWNLOAD_TIMEOUT);
    if (uc_byte == ACK) {
      prx_ymodem_read_buf(YMODEM_UART_PORT, &uc_byte, 1, DOWNLOAD_TIMEOUT);
      if (uc_byte == CRC16) {
        v_ymodem_send_packet_x(puc_tx_data, us_len, DOWNLOAD_TIMEOUT);

        x_file_done = USR_TRUE;
      }
    } else if ((uc_byte == CRC16) && (x_crc_nak == USR_TRUE)) {
      x_crc_nak = USR_FALSE;

      continue;
    } else if ((uc_byte != NAK) || (x_crc_nak == USR_TRUE)) {
      goto tx_err_handler;
    }
  } while (x_file_done == USR_FALSE);

  return us_len;

tx_err_handler:
  v_bsp_usart_byte_write(YMODEM_UART_PORT, CA);
  v_bsp_usart_byte_write(YMODEM_UART_PORT, CA);

  return 0;
}
