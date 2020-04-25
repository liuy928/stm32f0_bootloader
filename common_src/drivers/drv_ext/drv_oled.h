#ifndef __OLED_H
#define __OLED_H
#include "f0_systick_delay.h"
#include "sys_light_mode.h"

/***********************************宏定义************************************************/
#define OLED_CMD 0
#define OLED_DATA 1
#define OLED_MODE 0

#define OLED_MAX_COLUMN 128

/**********************************结构体定义**********************************************/
typedef enum X_OLED_POS_TYPE {
  OLED_POS_ABSOLUTE = 0,  //绝对坐标：输入时x对应绝对坐标0~128，y对应行数0~4
  OLED_POS_RELATIVE,  //相对坐标：与字体大小相对应，字体4*8显示32*4个字,字体8*16显示16*2个字
} xOledPosType_t;

typedef enum X_OLED_FONT_SIZE {
  FONT_SIZE_6X8 = 6,  // (字母或数字)字体大小：6*8像素,取x轴的宽度
  FONT_SIZE_8X16 = 8,  //  (字母或数字)字体大小：8*16像素,取x轴的宽度
} xOledFontSize_t;

typedef enum X_DISP_BACKGROUND_COLOR {
  DISP_BACK_COLOR_NORMAL = 0,  //正常显示,背景暗字体亮
  DISP_BACK_COLOR_INVERSE,     //反向显示，背景亮字体暗
} xDispBackColor_t;

typedef struct X_DISPLAY_NUMBER_HANDLE {
  float f_display_num;  // 显示的数字

  uint8_t uc_pos_x;           // 显示坐标x
  uint8_t uc_pos_y;           // 显示坐标y
  xOledPosType_t x_pos_type;  // 坐标类型

  uint8_t uc_integer_len;        // 整数部分长度
  uint8_t uc_decimal_precision;  // 小数精度

  uint8_t uc_show_percent;              // 是否在数字后显示百分号
  uint8_t uc_show_sign;                 // 是否显示+/-符号
  xOledFontSize_t x_font_size;          // 显示的字体大小
  xDispBackColor_t x_background_color;  // 显示背景色
} xDisplayNumberHandle_t;

typedef struct X_OLED_SCROll_BAR_HANDLE {
  const unsigned char *puc_string;  // 显示的字符串

  uint8_t uc_pos_x;           // 显示坐标x
  uint8_t uc_pos_y;           // 显示坐标y
  xOledPosType_t x_pos_type;  // 坐标类型

  uint8_t uc_display_len;  //显示的长度

  xOledFontSize_t x_font_size;  // 显示的字体大小
} x_oled_scroll_bar_handle_t;

/**********************************函数声明**********************************************/
extern void _v_drv_oled_write_byte(uint8_t uc_data, uint8_t uc_cmd);
extern void _v_drv_oled_set_pos(uint8_t uc_x, uint8_t uc_y);
extern void v_sys_oled_show_string(uint8_t x, uint8_t y, const char *chr,
                            xOledFontSize_t x_font_size);
extern void v_fxn_oled_show_integer(uint8_t uc_x, uint8_t uc_y, uint32_t ul_num,
                             uint8_t uc_len, xOledFontSize_t x_font_size,
                             uint8_t uc_if_show_0);
extern void v_fxn_oled_clear(void);

extern void vDrvOledInit(void);
extern void v_fxn_oled_show_char(uint8_t uc_x, uint8_t uc_y,
                                 unsigned char uc_char,
                                 xOledFontSize_t x_font_size);
extern void v_fxn_oled_show_symbol(uint8_t uc_x, uint8_t uc_y,
                                   uint8_t uc_symbol_index,
                                   xOledFontSize_t x_font_size);
extern void v_fxn_oled_display_update(xLightModeHandle_t *px_light_mode,
                                      float f_dim_level);
extern void v_fxn_scroll_bar_gel_text(uint8_t uc_string_change_flag);

#endif
