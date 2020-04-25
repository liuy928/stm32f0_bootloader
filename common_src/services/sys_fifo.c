/**************************************************************************
  @Company:
    ChromaTECH.

  @File Name:
    system_buffer.c

  @Description:

  @Author:
    liu928.

  @Mail:
    yu.liu@chromatechlighting.com

  @Time:         2018.09.04
  **************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************
#include "sys_fifo.h"

static xFifo_t *prx_fifo_init(uint8_t *puc_buffer, uint16_t us_size);

// @function 判断一个数是否为2的幂次；
// @para ul_n - 将要被判断的数；
// @return xUserBool_t - 结果；
xUserBool_t is_power_of_2(uint32_t ul_n)
{
  return (xUserBool_t)((ul_n != 0) && ((ul_n & (ul_n - 1)) == 0));
}

// @function 获取比给定值大的最小2的幂；
// @para ul_n - 将要被判断的数；
// @return uint32_t - 结果；
uint32_t ul_roundup_power_of_two(uint32_t ul_n)
{
  ul_n |= (ul_n >> 1);
  ul_n |= (ul_n >> 2);
  ul_n |= (ul_n >> 4);
  ul_n |= (ul_n >> 8);
  ul_n |= (ul_n >> 16);

  return (ul_n + 1);
}

// @function 创建特定缓存大小的FIFO，缓存只能以2的幂传入；
// @para us_size - 将要创建的FIFO容量，该参数最好是以2的幂传入，否则会自动扩展成2的幂；
// @return xFifo_t* - 创建好的FIFO的指针；
xFifo_t *px_fifo_create(uint16_t us_size)
{
  if(us_size == 0) {
    return NULL;
  }

  // 如果传入的长度不等于2的幂，则向上扩展成最小的2的幂；
  if(is_power_of_2(us_size) == USR_FALSE) {
    us_size = ul_roundup_power_of_two(us_size);
  }

  uint8_t *puc_buffer = (uint8_t *)calloc(us_size, sizeof(uint8_t));
  if (puc_buffer == NULL) {
    LOG_OUT_ERR("fifo creat failure...");

    return NULL;
  }

  xFifo_t *px_ret = prx_fifo_init(puc_buffer, us_size);
  if (px_ret == NULL) {
    free(puc_buffer);
  }

  return px_ret;
}

// @function 删除fifo，并释放空间；
// @para px_fifo - 将要删除的FIFO；
// @return 无；
void v_fifo_delete(xFifo_t *px_fifo)
{
  free(px_fifo->puc_buffer);
  free(px_fifo);
}

// @function 复位fifo；
// @para px_fifo - 将要复位的FIFO；
// @return 无；
void v_fifo_reset(xFifo_t *px_fifo)
{
  px_fifo->us_in = 0;
  px_fifo->us_out = 0;
}

// @function 获取FIFO内有效数据的长度；
uint16_t us_fifo_get_valid_length(xFifo_t *px_fifo)
{
  return (uint16_t)(px_fifo->us_in - px_fifo->us_out);
}

// @function 往FIFO里面存入一定长度的数据，并返回实际写入的数据长度；
// @para px_fifo - 要写入数据的FIFO；
// @para puc_buffer - 指向将要写入的数据的指针；
// @para us_len - 期望写入的数据长度；
// @return uint16_t - 实际写入的数据长度；
uint16_t us_fifo_put(xFifo_t *px_fifo, const unsigned char *puc_buffer,
                     uint16_t us_len)
{
  if ((px_fifo == NULL) || (puc_buffer == NULL)) {
    return 0;
  }

  us_len =
      USER_GET_MIN(us_len, (uint16_t)(px_fifo->us_size - px_fifo->us_in + px_fifo->us_out));
  uint16_t us_put_size = USER_GET_MIN(
      us_len, px_fifo->us_size - (px_fifo->us_in & (px_fifo->us_size - 1)));
  memcpy(px_fifo->puc_buffer + (px_fifo->us_in & (px_fifo->us_size - 1)),
         puc_buffer, us_put_size);
  memcpy(px_fifo->puc_buffer, puc_buffer + us_put_size, us_len - us_put_size);

  // if(px_fifo->us_in >= px_fifo->us_out) {
  //   us_put_size = USER_GET_MIN(us_len,
  //                              px_fifo->us_size - px_fifo->us_in + px_fifo->us_out);
  // } else if(px_fifo->us_in < px_fifo->us_out) {
  //   us_put_size = USER_GET_MIN(us_len,
  //                              px_fifo->us_out - px_fifo->us_in);
  // }

  // uint16_t us_put_tail = USER_GET_MIN(us_put_size, px_fifo->us_size - px_fifo->us_in);
  // memcpy(px_fifo->puc_buffer + px_fifo->us_in, puc_buffer, us_put_tail);
  // memcpy(px_fifo->puc_buffer, puc_buffer + us_put_tail, us_put_size - us_put_tail);

  px_fifo->us_in += us_len;
  // px_fifo->us_in %= px_fifo->us_size;

  return us_len;
}

// @function 从FIFO里面读取一定长度的数据，并返回实际读取的数据长度；
// @para px_fifo - 要读取数据的FIFO；
// @para puc_buffer - 指向读出的数据存放位置的指针；
// @para us_len - 期望读取的数据长度；
// @return uint16_t - 实际读取的数据长度；
uint16_t us_fifo_get(xFifo_t *px_fifo, const unsigned char *puc_buffer,
                     uint16_t us_len)
{
  if((px_fifo == NULL) || (puc_buffer == NULL)) {
    return 0;
  }

  us_len = USER_GET_MIN(us_len, (uint16_t)(px_fifo->us_in - px_fifo->us_out));
  uint16_t us_get_size = USER_GET_MIN(
      us_len, px_fifo->us_size - (px_fifo->us_out & (px_fifo->us_size - 1)));
  memcpy((void *)puc_buffer,
         px_fifo->puc_buffer + (px_fifo->us_out & (px_fifo->us_size - 1)),
         us_get_size);
  memcpy((void *)(puc_buffer + us_get_size), px_fifo->puc_buffer,
         us_len - us_get_size);

  px_fifo->us_out += us_len;

  return us_len;
}

// @function 初始化创建一个fifo
// @para puc_buffer - fifo缓存空间的指针；
// @para us_size - fifo缓存的大小
// @return xFifo_t - 初始化的FIFO的指针；
static xFifo_t *prx_fifo_init(uint8_t *puc_buffer, uint16_t us_size) {
  if(puc_buffer == NULL) {
    return NULL;
  }

  xFifo_t *px_fifo = malloc(sizeof(xFifo_t));
  if(px_fifo == NULL) {
    return NULL;
  }

  px_fifo->puc_buffer = puc_buffer;
  px_fifo->us_in = 0;
  px_fifo->us_out = 0;
  px_fifo->us_size = us_size;

  return px_fifo;
}
