/**************************************************************************
  Company:
    Self.
    
  File Name:
    drv_dac108s085.h

  Description:
    .                                                         
  **************************************************************************/

#ifndef __DRV_DAC108S085_H
#define __DRV_DAC108S085_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************
#include "system_includes.h"

#define  DAC_CH_MASK          0x0000 // 在WRM或WTM模式下，第15位为0，第14到第12位为通道编号，0~7对应A~H，第11到第2位为相应的DAC值
#define  CMD_MODE_WRM         0x8000
#define  CMD_MODE_WTM         0x9000
#define  CMD_UPDATE_SEL_CH    0xA000  //低八位分别对应A~H通道编号，最低位对应A 
#define  CMD_CH_A_WR          0xBFFC
#define  CMD_BOARDCAST        0xCFFC
#define  CMD_HIGH_Z_OUT       0xD000  //低八位分别对应A~H通道编号，最低位对应A 
#define  CMD_100K_OUT         0xE000  //
#define  CMD_2K5_OUT          0xF000  //

typedef enum{
  CHA = 0, CHB, CHC, CHD, CHE, CHF, CHG, CHH,
}DAC108S_CH_TABLE;

typedef enum{
  CHA1R1 = 0, CHA2G1, CHA3B1, CHA4W1, CHA5A1,
  CHB1R2, CHB2G2, CHB3B2, CHB4W2, CHB5A2,
  CHC1R3, CHC2G3, CHC3B3, CHC4W3, CHC5A3,
  CHD1R4, CHD2G4, CHD3B4, CHD4W4, CHD5A4,
  CHE1R5, CHE2G5, CHE3B5, CHE4W5, CHE5A5,
  CHF1R6, CHF2G6, CHF3B6, CHF4W6, CHF5A6,
  CHG1R7, CHG2G7, CHG3B7, CHG4W7, CHG5A7,
  CHH1R8, CHH2G8, CHH3B8, CHH4W8, CHH5A8,
}DAC108S_MATRIX_CH_INDEX;

extern uint16_t dac_ch_val_arr[8 * DAC108S_DEV_NUM];

#define dac108s_falsh_cs(a)   if(a)\
                              GPIO_SetBits(DAC108S_CS_PORT, DAC108S_CS_PIN);\
                              else\
                              GPIO_ResetBits(DAC108S_CS_PORT, DAC108S_CS_PIN)

extern void drv_dac108s085_cs_init(void);
extern void drv_dac108s085_mode_set(uint16_t mode);
extern void drv_dac108s085_single_dev_single_ch_output(DAC108S_CH_TABLE ch_index, uint16_t dac_val);
extern void drv_dac108s085_multi_dev_all_ch_output(uint16_t *dac_val);
extern void drv_dac108s085_test(void);

#endif // #ifndef __DRV_DAC108S085_H

