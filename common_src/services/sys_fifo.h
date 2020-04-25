/**************************************************************************
  @Company:
    ChromaTECH.

  @File Name:
    system_buffer.h

  @Description:

  @Author:
    abnerliu.

  @Mail:
    yu.liu@chromatechlighting.com

  @Time:         2018.09.04
  **************************************************************************/
#ifndef __SYS_FIFO_H
#define __SYS_FIFO_H

// *****************************************************************************
// Section: File includes
// *****************************************************************************
#include "system_platform_config.h"

typedef struct {
  uint8_t *puc_buffer;
  uint16_t us_in;
  uint16_t us_out;
  uint16_t us_size;
} xFifo_t;

extern xFifo_t *px_fifo_create(uint16_t us_size);

extern void v_fifo_delete(xFifo_t *px_fifo);
extern void v_fifo_reset(xFifo_t *px_fifo);

extern uint16_t us_fifo_get_valid_length(xFifo_t *px_fifo);

extern uint16_t us_fifo_put(xFifo_t *px_fifo, const uint8_t *puc_buffer, uint16_t us_len);
extern uint16_t us_fifo_get(xFifo_t *px_fifo, const uint8_t *puc_buffer, uint16_t us_len);

#endif // #ifndef __SYS_BUFFER_H