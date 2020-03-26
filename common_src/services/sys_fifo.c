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

static xFifo_t *prxFifoInit(uint8_t *puc_buffer, uint16_t us_size);

// @function 判断一个数是否为2的幂次；
// @para ul_n - 将要被判断的数；
// @return xUserBool_t - 结果；
boolean is_power_of_2(uint32_t ul_n) {
  return (boolean)(ul_n != 0 && ((ul_n & (ul_n - 1)) == 0));
}

// @function 获取比给定值大的最小2的幂；
// @para ul_n - 将要被判断的数；
// @return uint32_t - 结果；
uint32_t ul_roundup_power_of_two(uint32_t ul_n) {
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
xFifo_t *px_fifo_create(uint16_t us_size) {
  if(us_size == 0) {
    return NULL;
  }

#if 0
  if(is_power_of_2(us_size) == USR_FALSE) {
    us_size = ul_roundup_power_of_two(us_size);
  }
#endif // #if 0

  uint8_t *puc_buffer = (uint8_t*)malloc(us_size);
  if(puc_buffer == NULL) {
    LOG_OUT_ERR("fifo creat failure...");

    return NULL;
  }

  xFifo_t *px_ret = prxFifoInit(puc_buffer, us_size);
  if(px_ret == NULL) {
    free(puc_buffer);
  }

  return px_ret;
}

// @function 删除fifo，并释放空间；
// @para px_fifo - 将要删除的FIFO；
// @return 无；
void xFifoDelete(xFifo_t *px_fifo) {
  free(px_fifo->puc_buffer);
  px_fifo->us_in = 0;
  px_fifo->us_out = 0;
  px_fifo->us_size = 0;
}

void xFifoReset(xFifo_t *px_fifo) {
  px_fifo->us_in = 0;
  px_fifo->us_out = 0;
}

uint16_t us_fifo_put(xFifo_t *px_fifo,
                     const uint8_t *puc_buffer,
                     uint16_t us_len) {
  if((px_fifo == NULL) || (puc_buffer == NULL)) {
    return 0;
  }

  uint16_t us_put_size = 0;
  if(px_fifo->us_in >= px_fifo->us_out) {
    us_put_size = USER_GET_MIN(us_len,
                               px_fifo->us_size - px_fifo->us_in + px_fifo->us_out);
  } else if(px_fifo->us_in < px_fifo->us_out) {
    us_put_size = USER_GET_MIN(us_len,
                               px_fifo->us_out - px_fifo->us_in);
  }

  uint16_t us_put_tail = USER_GET_MIN(us_put_size, px_fifo->us_size - px_fifo->us_in);
  memcpy(px_fifo->puc_buffer + px_fifo->us_in, puc_buffer, us_put_tail);
  memcpy(px_fifo->puc_buffer, puc_buffer + us_put_tail, us_put_size - us_put_tail);

  px_fifo->us_in += us_put_size;
  px_fifo->us_in %= px_fifo->us_size;
#if 0
  px_fifo->us_in = px_fifo->us_in & (px_fifo->us_size - 1);
#endif // #if 0

  return us_put_size;
}

uint16_t us_fifo_get(xFifo_t *px_fifo, const uint8_t *puc_buffer, uint16_t us_len) {
  if((px_fifo == NULL) || (puc_buffer == NULL)) {
    return 0;
  }

  uint16_t us_get_size = 0;
  if(px_fifo->us_in > px_fifo->us_out) {
    us_get_size = USER_GET_MIN(us_len,
                               px_fifo->us_in - px_fifo->us_out);
  } else if(px_fifo->us_in < px_fifo->us_out) {
    us_get_size = USER_GET_MIN(us_len,
                               px_fifo->us_size + px_fifo->us_in - px_fifo->us_out);
  } else {
    return 0;
  }

  uint16_t us_get_tail = USER_GET_MIN(us_get_size, px_fifo->us_size - px_fifo->us_out);
  memcpy((void*)puc_buffer, px_fifo->puc_buffer + px_fifo->us_out, us_get_tail);
  memcpy((void*)(puc_buffer + us_get_tail), px_fifo->puc_buffer, us_get_size - us_get_tail);

  px_fifo->us_out += us_get_size;
  px_fifo->us_out %= px_fifo->us_size;
#if 0
  px_fifo->us_out = px_fifo->us_out & (px_fifo->us_size - 1);
#endif // #if 0

  return us_get_size;
}

// @function 初始化创建一个fifo
// @para puc_buffer - fifo缓存空间的指针；
// @para us_size - fifo缓存的大小
// @return xFifo_t - 初始化的FIFO的指针；
static xFifo_t *prxFifoInit(uint8_t *puc_buffer, uint16_t us_size) {
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
