#include "drv_oled.h"
#include "drv_oled_font.h"
#include "f0_bsp_iic.h"
#include "f0_systick_delay.h"
#include "sys_light_mode.h"
#include "sys_text.h"

x_oled_scroll_bar_handle_t x_scroll_bar_gel_list = {
    //.puc_string = puc_gel_list_table[0],
    .uc_pos_x = 0,
    .uc_pos_y = 2,
    .x_pos_type = OLED_POS_ABSOLUTE,
    .uc_display_len = 16,
    .x_font_size = FONT_SIZE_8X16,
};

// SSD1306 driver
void vDrvOledInit(void) {
  vSystemDelayXms(100);
  _v_drv_oled_write_byte(0xAE, OLED_CMD);  //显示开关：AE关闭显示，AF打开显示

  _v_drv_oled_write_byte(0x40, OLED_CMD);
  _v_drv_oled_write_byte(0xB0, OLED_CMD);

  _v_drv_oled_write_byte(0xC8, OLED_CMD);

  _v_drv_oled_write_byte(0x81, OLED_CMD);
  _v_drv_oled_write_byte(0xff, OLED_CMD);

  _v_drv_oled_write_byte(0xa1, OLED_CMD);

  _v_drv_oled_write_byte(0xa6, OLED_CMD);  //反白开关：A6正常显示，A7反白显示

  _v_drv_oled_write_byte(0xa8, OLED_CMD);
  _v_drv_oled_write_byte(0x1f, OLED_CMD);

  _v_drv_oled_write_byte(0xd3, OLED_CMD);
  _v_drv_oled_write_byte(0x00, OLED_CMD);

  _v_drv_oled_write_byte(0xd5, OLED_CMD);
  _v_drv_oled_write_byte(0xf0, OLED_CMD);

  _v_drv_oled_write_byte(0xd9, OLED_CMD);
  _v_drv_oled_write_byte(0x22, OLED_CMD);

  _v_drv_oled_write_byte(0xda, OLED_CMD);
  _v_drv_oled_write_byte(0x02, OLED_CMD);

  _v_drv_oled_write_byte(0xdb, OLED_CMD);
  _v_drv_oled_write_byte(0x49, OLED_CMD);

  _v_drv_oled_write_byte(0x8d, OLED_CMD);
  _v_drv_oled_write_byte(0x14, OLED_CMD);

  _v_drv_oled_write_byte(0xaf, OLED_CMD);
  v_fxn_oled_clear();
}

void _v_drv_oled_write_byte(uint8_t uc_data, uint8_t uc_cmd) {
  vBspIicSendStart();

  vBspIicWriteByte(0x78);
  if (vBspIicWaitAck()) {
    return;
  }

  if (uc_cmd) {
    vBspIicWriteByte(0x40);
  } else {
    vBspIicWriteByte(0x00);
  }
  if (vBspIicWaitAck()) {
    return;
  }

  vBspIicWriteByte(uc_data);
  if (vBspIicWaitAck()) {
    return;
  }

  vBspIicSendStop();
}

void _v_drv_oled_set_pos(uint8_t uc_x, uint8_t uc_y) {
  _v_drv_oled_write_byte(0xB0 + uc_y, OLED_CMD);
  _v_drv_oled_write_byte(((uc_x & 0xF0) >> 4) | 0x10, OLED_CMD);
  _v_drv_oled_write_byte((uc_x & 0x0F), OLED_CMD);
}

void v_fxn_oled_clear(void) {
  uint8_t uc_i, uc_n;

  for (uc_i = 0; uc_i < 8; uc_i++) {
    _v_drv_oled_write_byte(0xb0 + uc_i, OLED_CMD);
    _v_drv_oled_write_byte(0x00, OLED_CMD);
    _v_drv_oled_write_byte(0x10, OLED_CMD);

    for (uc_n = 0; uc_n < 128; uc_n++) {
      _v_drv_oled_write_byte(0, OLED_DATA);
    }
  }
}

/*
uint8_t x:坐标x轴，左上起往右累加,绝对坐标，范围：0~128
uint8_t y:行数，范围0~4行
uint8_t uc_char:显示的字符
xOledFontSize_t x_font_size:显示字符的高度
*/
void v_fxn_oled_show_char(uint8_t uc_x, uint8_t uc_y, unsigned char uc_char,
                          xOledFontSize_t x_font_size) {
  uint8_t uc_i = 0;
  uint8_t uc_char_index = uc_char - ' ';

  if (uc_x > OLED_MAX_COLUMN - 1) {
    return;
  }

  if (x_font_size == FONT_SIZE_8X16) {
    _v_drv_oled_set_pos(uc_x, uc_y);

    //显示前8个字节，即字符的上半部分
    for (uc_i = 0; uc_i < 8; uc_i++) {
      _v_drv_oled_write_byte(puc_oled_font_8x16[uc_char_index][uc_i],
                             OLED_DATA);
    }

    //显示后8个字节，即字符的下半部分
    _v_drv_oled_set_pos(uc_x, uc_y + 1);

    for (uc_i = 0; uc_i < 8; uc_i++) {
      _v_drv_oled_write_byte(puc_oled_font_8x16[uc_char_index][uc_i + 8],
                             OLED_DATA);
    }
  } else {
    _v_drv_oled_set_pos(uc_x, uc_y);

    for (uc_i = 0; uc_i < 6; uc_i++) {
      _v_drv_oled_write_byte(puc_oled_font_6x8[uc_char_index][uc_i], OLED_DATA);
    }
  }
}

void v_fxn_oled_show_symbol(uint8_t uc_x, uint8_t uc_y, uint8_t uc_symbol_index,
                            xOledFontSize_t x_font_size) {
  uint8_t uc_i = 0;

  if (uc_x > OLED_MAX_COLUMN - 1) {
    return;
  }

  if (x_font_size == FONT_SIZE_8X16) {
    _v_drv_oled_set_pos(uc_x, uc_y);

    for (uc_i = 0; uc_i < 8; uc_i++) {
      _v_drv_oled_write_byte(puc_oled_symbol_8x16[uc_symbol_index][uc_i],
                             OLED_DATA);
    }

    _v_drv_oled_set_pos(uc_x, uc_y + 1);

    for (uc_i = 0; uc_i < 8; uc_i++) {
      _v_drv_oled_write_byte(puc_oled_symbol_8x16[uc_symbol_index][uc_i + 8],
                             OLED_DATA);
    }
  }
}

//指数计算，m基数，n指数
uint32_t _ul_fxn_exponent_cnt(uint8_t uc_m, uint8_t uc_n) {
  uint32_t result = 1;

  while (uc_n--) {
    result *= uc_m;
  }
  return result;
}

void v_fxn_oled_show_integer(uint8_t uc_x, uint8_t uc_y, uint32_t ul_num,
                             uint8_t uc_len, xOledFontSize_t x_font_size,
                             uint8_t uc_if_show_0) {
  uint8_t uc_i, uc_tmp;
  uint8_t enshow = 0;
  for (uc_i = 0; uc_i < uc_len; uc_i++) {
    uc_tmp = (ul_num / _ul_fxn_exponent_cnt(10, uc_len - uc_i - 1)) %
             10;  // 取余，得到某一位的值
    if ((enshow == 0) && (uc_i < (uc_len - 1)) &&
        (!uc_if_show_0))  //判断是否高位显示0
    {
      if (uc_tmp == 0) {
        v_fxn_oled_show_char(uc_x + x_font_size * uc_i, uc_y, ' ',
                             x_font_size);  //高位为0时显示空白
        continue;
      } else {
        enshow = 1;
      }
    }
    v_fxn_oled_show_char(uc_x + x_font_size * uc_i, uc_y, uc_tmp + '0',
                         x_font_size);
  }
}

void v_fxn_oled_display_num(xDisplayNumberHandle_t *px_display_num) {
  uint8_t uc_x, uc_y, uc_x_point, uc_x_decimal, uc_x_percent = 0;
  uint8_t uc_num_bit = 0;
  uint32_t l_temp_value;

  //坐标换算成绝对坐标
  if (OLED_POS_ABSOLUTE == px_display_num->x_pos_type) {
    if (!px_display_num->uc_show_sign) {
      uc_x = px_display_num->uc_pos_x;
    } else {
      uc_x = px_display_num->uc_pos_x + px_display_num->x_font_size;
    }
    uc_y = px_display_num->uc_pos_y;
  } else {
    if (!px_display_num->uc_show_sign) {
      uc_x = px_display_num->x_font_size * px_display_num->uc_pos_x;
    } else {
      uc_x = px_display_num->x_font_size * (px_display_num->uc_pos_x + 1);
    }
    if (FONT_SIZE_8X16 == px_display_num->x_font_size) {
      uc_y = px_display_num->uc_pos_y * 2;
    } else if (FONT_SIZE_6X8 == px_display_num->x_font_size) {
      uc_y = px_display_num->uc_pos_y;
    }
  }

  //小数精度
  if (px_display_num->uc_decimal_precision) {
    uc_x_point =
        uc_x + px_display_num->x_font_size * px_display_num->uc_integer_len;
    uc_x_decimal = uc_x_point + px_display_num->x_font_size;
    uc_x_percent = uc_x_decimal + px_display_num->x_font_size *
                                      px_display_num->uc_decimal_precision;
  } else {
    uc_x_percent =
        uc_x + px_display_num->x_font_size * px_display_num->uc_integer_len;
  }

  //显示正负号
  if (px_display_num->uc_show_sign) {
    if (px_display_num->f_display_num >= 0) {
      v_fxn_oled_show_char(uc_x - px_display_num->x_font_size, uc_y, '+',
                           px_display_num->x_font_size);
    } else {
      v_fxn_oled_show_char(uc_x - px_display_num->x_font_size, uc_y, '-',
                           px_display_num->x_font_size);
      px_display_num->f_display_num *= -1.0f;
    }
  }

  //显示整数部分
  for (uc_num_bit = 0; uc_num_bit < px_display_num->uc_integer_len;
       uc_num_bit++) {
    v_fxn_oled_show_integer(uc_x, uc_y, (uint32_t)px_display_num->f_display_num,
                            px_display_num->uc_integer_len,
                            px_display_num->x_font_size, 0);
  }

  //显示小数部分
  if (px_display_num->uc_decimal_precision) {
    l_temp_value = (uint32_t)(
        px_display_num->f_display_num *
        _ul_fxn_exponent_cnt(10, px_display_num->uc_decimal_precision));

    v_fxn_oled_show_char(uc_x_point, uc_y, '.', px_display_num->x_font_size);

    v_fxn_oled_show_integer(uc_x_decimal, uc_y, l_temp_value,
                            px_display_num->uc_decimal_precision,
                            px_display_num->x_font_size, 1);
  }

  //显示百分号
  if (px_display_num->uc_show_percent) {
    if (px_display_num->uc_decimal_precision == 0) {
      v_fxn_oled_show_char(uc_x_percent, uc_y, '%',
                           px_display_num->x_font_size);
    } else {
      v_fxn_oled_show_char(uc_x_percent, uc_y, '%',
                           px_display_num->x_font_size);
    }
  }
}

void v_sys_oled_show_string(uint8_t uc_x, uint8_t uc_y, const char *puc_string,
                            xOledFontSize_t x_font_size) {
  uint8_t uc_i = 0;
  uint8_t _dis_x = uc_x, _dis_y = uc_y;

  if(_dis_y == 0) {
    while (puc_string[uc_i] != '\0') {
      v_fxn_oled_show_char(_dis_x, _dis_y, puc_string[uc_i], x_font_size);
      _dis_x += x_font_size;

      if ((uc_x + x_font_size) > 128) {  //超出当前行显示范围
        _dis_x = 0;
        _dis_y = 2;
      }

      uc_i++;
      if(puc_string[uc_i] == '\n') {
        _dis_x = 0;
        _dis_y = 2;
        uc_i++;
      }
    }
  } else {
    while (puc_string[uc_i] != '\0') {
      v_fxn_oled_show_char(uc_x, uc_y, puc_string[uc_i], x_font_size);
      uc_x += x_font_size;

      if ((uc_x + x_font_size) > 128) {  //超出当前行显示范围
        return;
      }

      uc_i++;
    }
  }
}

void v_fxn_text_scroll_bar(x_oled_scroll_bar_handle_t *x_scroll_bar_handle,
                           const char *pc_test_string) {
  static uint8_t uc_time_acc = 0;
  char const *p = pc_test_string;

  if (x_scroll_bar_handle->uc_display_len ==
      (sizeof(x_scroll_bar_handle->puc_string[uc_time_acc]) - uc_time_acc)) {
    uc_time_acc = 0;
  }
  v_sys_oled_show_string(
      x_scroll_bar_handle->uc_pos_x, x_scroll_bar_handle->uc_pos_y,
      (void *)(*p + uc_time_acc), x_scroll_bar_handle->x_font_size);
}

void v_fxn_oled_display_update(xLightModeHandle_t *px_light_mode,
                               float f_dim_level) {
  float f_relative_para3 = 0.0f;
  static xLightModeType_t x_light_mode_last_status = LIGHT_MODE_UNKNOWN;

  static float uc_dim_last_status = 0;
  static uint16_t us_para2_last_status = 0;
  static int16_t c_para3_last_status = 0;

  char pc_string[64];

  xDisplayNumberHandle_t *px_oled_num =
      (xDisplayNumberHandle_t *)malloc(sizeof(xDisplayNumberHandle_t));

  switch (px_light_mode->x_index) {
    case LIGHT_MODE_HSI:
      if (LIGHT_MODE_HSI != x_light_mode_last_status) {
        v_fxn_oled_clear();

        v_sys_oled_show_string(0, 0, "HSI     Dim:", FONT_SIZE_8X16);
        v_sys_oled_show_string(0, 2, "Hue:    Sat:", FONT_SIZE_8X16);
        v_fxn_oled_show_symbol(57, 2, 0x00, FONT_SIZE_8X16);
      }

      if (px_light_mode->x_index != x_light_mode_last_status) {
        // 显示DIM
        if (f_dim_level != uc_dim_last_status) {
          px_oled_num->f_display_num = f_dim_level * 100;

          px_oled_num->uc_pos_x = 12;
          px_oled_num->uc_pos_y = 0;
          px_oled_num->uc_decimal_precision = 0;
          px_oled_num->uc_integer_len = 3;
          px_oled_num->x_pos_type = OLED_POS_RELATIVE;
          px_oled_num->x_font_size = FONT_SIZE_8X16;
          px_oled_num->uc_show_percent = 1;
          px_oled_num->uc_show_sign = 0;
          v_fxn_oled_display_num(px_oled_num);

          uc_dim_last_status = f_dim_level;
        }

        //显示HUE
        if (px_light_mode->us_para2 != us_para2_last_status) {
            px_oled_num->f_display_num = (uint32_t)px_light_mode->us_para2;
            px_oled_num->uc_pos_x = 4;
            px_oled_num->uc_pos_y = 1;
            px_oled_num->uc_decimal_precision = 0;
            px_oled_num->uc_integer_len = 3;
            px_oled_num->x_pos_type = OLED_POS_RELATIVE;
            px_oled_num->x_font_size = FONT_SIZE_8X16;
            px_oled_num->uc_show_percent = 0;
            px_oled_num->uc_show_sign = 0;
            v_fxn_oled_display_num(px_oled_num);

            us_para2_last_status = px_light_mode->us_para2;
          }

        //显示SAT
        if (px_light_mode->s_para3 != c_para3_last_status) {
          px_oled_num->f_display_num = px_light_mode->s_para3;
          px_oled_num->uc_pos_x = 12;
          px_oled_num->uc_pos_y = 1;
          px_oled_num->uc_decimal_precision = 0;
          px_oled_num->uc_integer_len = 3;
          px_oled_num->x_pos_type = OLED_POS_RELATIVE;
          px_oled_num->x_font_size = FONT_SIZE_8X16;
          px_oled_num->uc_show_percent = 1;
          px_oled_num->uc_show_sign = 0;
          v_fxn_oled_display_num(px_oled_num);

          c_para3_last_status = px_light_mode->s_para3;
        }
      }

      x_light_mode_last_status = px_light_mode->x_index;

      break;

    case LIGHT_MODE_CCT:
      // v_fxn_oled_clear();

      // 显示第一行；
      memset(pc_string, '\0', 64);
      sprintf(pc_string, "CCT   DIM:%d.%d%%\n%dK  G/M:%+d.%d%",
              (uint16_t)f_dim_level / 10, (uint16_t)f_dim_level % 20,
              px_light_mode->us_para2 * 50 + 2700,
              px_light_mode->s_para3 / 100, abs(px_light_mode->s_para3) % 100);
      v_sys_oled_show_string(0, 0, pc_string, FONT_SIZE_8X16);

//      if (LIGHT_MODE_CCT != x_light_mode_last_status) {
//        v_fxn_oled_clear();
//
//        v_sys_oled_show_string(0, 0, "CCT    Dim:", FONT_SIZE_8X16);
//        v_sys_oled_show_string(0, 2, "       G/M:", FONT_SIZE_8X16);
//      }
//
//      // 显示DIM
//      if ((f_dim_level != uc_dim_last_status) ||
//          (px_light_mode->x_index != x_light_mode_last_status)) {
//        px_oled_num->f_display_num = (uint32_t)(f_dim_level * 1.0 / 10);
//        px_oled_num->uc_pos_x = 11;
//        px_oled_num->uc_pos_y = 0;
//        px_oled_num->uc_decimal_precision = 0;
//        px_oled_num->uc_integer_len = 3;
//        px_oled_num->x_pos_type = OLED_POS_RELATIVE;
//        px_oled_num->x_font_size = FONT_SIZE_8X16;
//        px_oled_num->uc_show_percent = 1;
//        px_oled_num->uc_show_sign = 0;
//        v_fxn_oled_display_num(px_oled_num);
//
//        uc_dim_last_status = f_dim_level;
//      }
//
//      //显示CCT
//      if ((px_light_mode->us_para2 != us_para2_last_status) ||
//          (px_light_mode->x_index != x_light_mode_last_status)) {
//        px_oled_num->f_display_num =
//            (uint32_t)(px_light_mode->us_para2 * 50 + 2700);
//        px_oled_num->uc_pos_x = 0;
//        px_oled_num->uc_pos_y = 1;
//        px_oled_num->uc_decimal_precision = 0;
//
//        px_oled_num->x_pos_type = OLED_POS_RELATIVE;
//        px_oled_num->x_font_size = FONT_SIZE_8X16;
//        px_oled_num->uc_show_percent = 0;
//        px_oled_num->uc_show_sign = 0;
//
//        if (px_oled_num->f_display_num == 10000) {
//          px_oled_num->uc_integer_len = 5;
//          v_sys_oled_show_string(40, 2, "K", FONT_SIZE_8X16);
//        } else {
//          px_oled_num->uc_integer_len = 4;
//          v_sys_oled_show_string(32, 2, "K ", FONT_SIZE_8X16);
//        }
//        v_fxn_oled_display_num(px_oled_num);
//
//        us_para2_last_status = px_light_mode->us_para2;
//      }
//
//      //显示G/M
//      if ((px_light_mode->s_para3 != c_para3_last_status) ||
//          (px_light_mode->x_index != x_light_mode_last_status)) {
//        if (px_light_mode->s_para3 >= 0) {
//          f_relative_para3 =
//              (float)px_light_mode->s_para3 / px_light_mode->s_max_para3;
//        } else if (px_light_mode->s_para3 < 0) {
//          f_relative_para3 =
//              (float)px_light_mode->s_para3 / px_light_mode->s_max_para3;
//        }
//
//        px_oled_num->f_display_num = f_relative_para3;
//        px_oled_num->uc_pos_x = 11;
//        px_oled_num->uc_pos_y = 1;
//        px_oled_num->uc_decimal_precision = 2;
//        px_oled_num->uc_integer_len = 1;
//        px_oled_num->x_pos_type = OLED_POS_RELATIVE;
//        px_oled_num->x_font_size = FONT_SIZE_8X16;
//        px_oled_num->uc_show_percent = 0;
//        px_oled_num->uc_show_sign = 1;
//        v_fxn_oled_display_num(px_oled_num);
//
//        c_para3_last_status = px_light_mode->s_para3;
//      }
//
//      if (px_light_mode->x_index != x_light_mode_last_status) {
//        x_light_mode_last_status = px_light_mode->x_index;
//      }

      break;

    case LIGHT_MODE_SURFACE:
      if (LIGHT_MODE_SURFACE != x_light_mode_last_status) {
        v_fxn_oled_clear();

        v_sys_oled_show_string(0, 0, "SKIN    Dim:", FONT_SIZE_8X16);
        v_sys_oled_show_string(0, 2, "S:      Fre:", FONT_SIZE_8X16);
      }

      // 显示DIM
      if ((f_dim_level != uc_dim_last_status) ||
          (px_light_mode->x_index != x_light_mode_last_status)) {
        px_oled_num->f_display_num = (uint32_t)(f_dim_level * 100);
        px_oled_num->uc_pos_x = 12;
        px_oled_num->uc_pos_y = 0;
        px_oled_num->uc_decimal_precision = 0;
        px_oled_num->uc_integer_len = 3;
        px_oled_num->x_pos_type = OLED_POS_RELATIVE;
        px_oled_num->x_font_size = FONT_SIZE_8X16;
        px_oled_num->uc_show_percent = 1;
        px_oled_num->uc_show_sign = 0;
        v_fxn_oled_display_num(px_oled_num);

        uc_dim_last_status = f_dim_level;
      }

      //显示Soft
      if ((px_light_mode->us_para2 != us_para2_last_status) ||
          (px_light_mode->x_index != x_light_mode_last_status)) {
        px_oled_num->f_display_num = (uint32_t)px_light_mode->us_para2 / 1.46f;
        px_oled_num->uc_pos_x = 2;
        px_oled_num->uc_pos_y = 1;
        px_oled_num->uc_decimal_precision = 0;
        px_oled_num->uc_integer_len = 3;
        px_oled_num->x_pos_type = OLED_POS_RELATIVE;
        px_oled_num->x_font_size = FONT_SIZE_8X16;
        px_oled_num->uc_show_percent = 1;
        px_oled_num->uc_show_sign = 0;
        v_fxn_oled_display_num(px_oled_num);

        us_para2_last_status = px_light_mode->us_para2;
      }

      //显示Fresh
      if ((px_light_mode->s_para3 != c_para3_last_status) ||
          (px_light_mode->x_index != x_light_mode_last_status)) {
        px_oled_num->f_display_num = (px_light_mode->s_para3 + 100.0f) / 2.0f;
        px_oled_num->uc_pos_x = 12;
        px_oled_num->uc_pos_y = 1;
        px_oled_num->uc_decimal_precision = 0;
        px_oled_num->uc_integer_len = 3;
        px_oled_num->x_pos_type = OLED_POS_RELATIVE;
        px_oled_num->x_font_size = FONT_SIZE_8X16;
        px_oled_num->uc_show_percent = 1;
        px_oled_num->uc_show_sign = 0;
        v_fxn_oled_display_num(px_oled_num);

        c_para3_last_status = px_light_mode->s_para3;
      }

      if (px_light_mode->x_index != x_light_mode_last_status) {
        x_light_mode_last_status = px_light_mode->x_index;
      }

      break;

    case LIGHT_MODE_GEL:
      if (LIGHT_MODE_GEL != x_light_mode_last_status) {
        v_fxn_oled_clear();

        v_sys_oled_show_string(0, 0, "        Dim:", FONT_SIZE_8X16);
      }

      // 显示DIM
      if ((f_dim_level != uc_dim_last_status) ||
          (px_light_mode->x_index != x_light_mode_last_status)) {
        px_oled_num->f_display_num = (uint32_t)(f_dim_level * 100);
        px_oled_num->uc_pos_x = 12;
        px_oled_num->uc_pos_y = 0;
        px_oled_num->uc_decimal_precision = 0;
        px_oled_num->uc_integer_len = 3;
        px_oled_num->x_pos_type = OLED_POS_RELATIVE;
        px_oled_num->x_font_size = FONT_SIZE_8X16;
        px_oled_num->uc_show_percent = 1;
        px_oled_num->uc_show_sign = 0;
        v_fxn_oled_display_num(px_oled_num);

        uc_dim_last_status = f_dim_level;
      }

      //显示列表
      if ((px_light_mode->us_para2 != us_para2_last_status) ||
          (px_light_mode->x_index != x_light_mode_last_status)) {
        // v_sys_oled_show_string(0, 2,
        // puc_gel_list_table[px_light_mode->us_para2], 16);

        // v_sys_oled_show_string(24, 0, "/", 16);
        if (px_light_mode->us_para2 <= 121) {
          v_sys_oled_show_string(0, 0, "ROSCO", FONT_SIZE_8X16);
        } else {
          v_sys_oled_show_string(0, 0, "LEE  ", FONT_SIZE_8X16);
        }
        us_para2_last_status = px_light_mode->us_para2;

        v_fxn_scroll_bar_gel_text(1);
      }

      //			//显示色温
      //			if((px_light_mode->s_para3 !=
      // c_para3_last_status) || (px_light_mode->x_index !=
      // x_light_mode_last_status)) {
      // if(px_light_mode->s_para3 == 0) { 					v_sys_oled_show_string(88, 2, "3200K",
      //16); 				} else {
      //v_sys_oled_show_string(88, 2, "5600K", 16);
      //				}
      //
      //				c_para3_last_status =
      // px_light_mode->s_para3;
      //			}

      if (px_light_mode->x_index != x_light_mode_last_status) {
        x_light_mode_last_status = px_light_mode->x_index;
      }

      break;
    case LIGHT_MODE_EFFECT:
      if (LIGHT_MODE_EFFECT != x_light_mode_last_status) {
        v_fxn_oled_clear();

        v_sys_oled_show_string(0, 0, "EFFECT Dim:", FONT_SIZE_8X16);
        v_sys_oled_show_string(0, 2, "Speed", FONT_SIZE_8X16);
      }

      // 显示DIM
      if ((f_dim_level != uc_dim_last_status) ||
          (px_light_mode->x_index != x_light_mode_last_status)) {
        px_oled_num->f_display_num = (uint32_t)(f_dim_level * 100);
        px_oled_num->uc_pos_x = 11;
        px_oled_num->uc_pos_y = 0;
        px_oled_num->uc_decimal_precision = 0;
        px_oled_num->uc_integer_len = 3;
        px_oled_num->x_pos_type = OLED_POS_RELATIVE;
        px_oled_num->x_font_size = FONT_SIZE_8X16;
        px_oled_num->uc_show_percent = 1;
        px_oled_num->uc_show_sign = 0;
        v_fxn_oled_display_num(px_oled_num);

        uc_dim_last_status = f_dim_level;
      }

      //显示SPEED
      if ((px_light_mode->us_para2 != us_para2_last_status) ||
          (px_light_mode->x_index != x_light_mode_last_status)) {
        px_oled_num->f_display_num = (uint32_t)px_light_mode->us_para2 + 1;
        px_oled_num->uc_pos_x = 5;
        px_oled_num->uc_pos_y = 1;
        px_oled_num->uc_decimal_precision = 0;
        px_oled_num->uc_integer_len = 1;
        px_oled_num->x_pos_type = OLED_POS_RELATIVE;
        px_oled_num->x_font_size = FONT_SIZE_8X16;
        px_oled_num->uc_show_percent = 0;
        px_oled_num->uc_show_sign = 0;
        v_fxn_oled_display_num(px_oled_num);

        us_para2_last_status = px_light_mode->us_para2;
      }

      //显示Lighting Effect List
      if ((px_light_mode->s_para3 != c_para3_last_status) ||
          (px_light_mode->x_index != x_light_mode_last_status)) {
        v_sys_oled_show_string(
            56, 2, pc_lighting_effect_list_table[px_light_mode->s_para3],
            FONT_SIZE_8X16);

        c_para3_last_status = px_light_mode->s_para3;
      }

      if (px_light_mode->x_index != x_light_mode_last_status) {
        x_light_mode_last_status = px_light_mode->x_index;
      }
      break;

    default:
      return;
      break;
  }

  free(px_oled_num);
  px_oled_num = NULL;
}

void v_fxn_scroll_bar_gel_text(uint8_t uc_string_change_flag) {
  static uint8_t uc_light_mode_change_status = 0;

  if ((x_system_status == STATUS_NORMAL) &&
      (x_fxn_light_mode_index_get() == LIGHT_MODE_GEL)) {
    char *pc_text_string_temp = (char *)malloc(sizeof(char) * 40);
    uint8_t uc_string_len = 0;
    static uint8_t uc_scroll_acc = 0;
    static uint8_t uc_pause_acc = 0;
    static uint8_t uc_pause_status = 0;

    xLightModeHandle_t *px_light_mode =
        px_fxn_light_mode_handle_get(LIGHT_MODE_GEL);
    //计算字符串长度
    uc_string_len = strlen(pc_gel_list_table[px_light_mode->us_para2]);

    if (uc_string_change_flag) {
      uc_string_change_flag = 0;
      uc_scroll_acc = 0;
      uc_pause_status = 0;
    }

    if (uc_light_mode_change_status) {
      uc_light_mode_change_status = 0;
      uc_scroll_acc = 0;
      uc_pause_status = 0;
    }

    switch (uc_pause_status) {
      case 0:
        if ((3 < (uc_pause_acc++)) && (16 < uc_string_len)) {
          uc_pause_status = 1;
          uc_pause_acc = 0;
        }

        memset(pc_text_string_temp, ' ', sizeof(char) * 40);
        //将文本赋给缓存
        memcpy(pc_text_string_temp, pc_gel_list_table[px_light_mode->us_para2],
               uc_string_len);

        break;

      case 1:
        if (16 > (uc_string_len - uc_scroll_acc)) {
          if (2 < (uc_pause_acc++)) {
            uc_pause_status = 0;
            uc_scroll_acc = 0;
          }
        } else {
          memset(pc_text_string_temp, ' ', sizeof(char) * 40);
          //将文本左移uc_acc个字符，赋给缓存
          memcpy(pc_text_string_temp,
                 pc_gel_list_table[px_light_mode->us_para2] + uc_scroll_acc,
                 uc_string_len - uc_scroll_acc);

          uc_scroll_acc++;
        }
        break;

      default:
        break;
    }

    v_sys_oled_show_string(0, 2, pc_text_string_temp,
                           FONT_SIZE_8X16);  //显示当前文本

    free(pc_text_string_temp);
    pc_text_string_temp = NULL;
  } else {
    uc_light_mode_change_status = 1;
  }
}

// void v_fxn_scroll_bar_gel_text(void) {
//	static uint8_t uc_light_mode_change_status = 0;
//
//	if(x_fxn_light_mode_index_get() == LIGHT_MODE_GEL) {
//		const char *pc_text_string = NULL;
//		char *pc_text_string_temp = (char*)malloc(sizeof(char) * 40);
//		static uint8_t uc_acc = 0;
//		uint8_t uc_string_len = 0;
//		static uint8_t uc_static = 0;
//
//		xLightModeHandle_t *px_light_mode =
// px_fxn_light_mode_handle_get(LIGHT_MODE_GEL); 		pc_text_string =
// puc_gel_list_table[px_light_mode->us_para2];//获取显示的字符串
// uc_string_len = strlen(pc_text_string);//计算字符串长度
//
//		if(uc_light_mode_change_status) {
//			uc_light_mode_change_status = 0;
//			uc_acc = 0;
//		}
//
//		if(16 >= (uc_string_len - uc_acc)) {
//			memset(pc_text_string_temp, ' ', sizeof(char) * 40);
//			memcpy(pc_text_string_temp, pc_text_string,
// uc_string_len);//将文本赋给缓存 			uc_acc = 0; 		} else
// {
//      uc_acc++;
//			memset(pc_text_string_temp, ' ', sizeof(char) * 40);
//			memcpy(pc_text_string_temp, pc_text_string + uc_acc,
// uc_string_len - uc_acc);//将文本左移uc_acc个字符，赋给缓存
//    }
//
//		v_sys_oled_show_string(0, 2, pc_text_string_temp,
//FONT_SIZE_8X16);
////显示当前文本
//
//		free(pc_text_string_temp);
//		pc_text_string_temp = NULL;
//	} else {
//		uc_light_mode_change_status = 1;
//	}
//}
