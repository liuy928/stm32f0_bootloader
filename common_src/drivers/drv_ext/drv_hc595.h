/**************************************************************************
  Company:
    Self.
    
  File Name:
    drv_hc595.h

  Description:
    .                                                         
  **************************************************************************/

#ifndef __DRV_HC595_H
#define __DRV_HC595_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************
#include "system_includes.h"

#define _drv_hc595_ds_out(a) 		if(a)\
                                GPIO_SetBits(DRV_HC595_PORT, DRV_HC595_DS);\
                                else\
                                GPIO_ResetBits(DRV_HC595_PORT, DRV_HC595_DS)

#define _bsp_hc595_shift_out(a) if(a)\
                                GPIO_SetBits(DRV_HC595_PORT, DRV_HC595_SHIFT);\
                                else\
                                GPIO_ResetBits(DRV_HC595_PORT, DRV_HC595_SHIFT)
      
#define _bsp_hc595_load_out(a)  if(a)\
                                GPIO_SetBits(DRV_HC595_PORT, DRV_HC595_LOAD);\
                                else\
                                GPIO_ResetBits(DRV_HC595_PORT, DRV_HC595_LOAD)

extern volatile uint8_t hc595_control_matrix[5];
                                  
extern void drv_hc595_initialize(void);                                  
extern void drv_hc595_update(uint8_t *ptr);                                  
extern void drv_hc595_bit_control(uint8_t bit_index, FlagStatus status);

#endif // #ifndef __DRV_HC595_H