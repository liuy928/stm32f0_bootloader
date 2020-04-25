
#include "f0_bsp_flash.h"

// @function 从flash读取多个板半字；
// @para addr - 读取首地址；
// @para destination - 读出数据目标存放地址；
// @para num - 读取的半字数
void bsp_flash_read_n_hword(uint32_t addr, void *destination, uint16_t num)
{
  if (destination == NULL) return;

  uint16_t *data = (uint16_t *)destination;

  for (uint16_t i = 0; i < num; i++) {
    data[i] = *(__IO uint16_t *)addr;
    addr += 2;
  }
}

// @function 往flash写入多个板半字，不做擦除检查；
// @para addr - 写入首地址；
// @para destination - 要写入的数据目标存放地址；
// @para num - 写入的半字数
HAL_StatusTypeDef bsp_flash_write_n_hword_nocheck(uint32_t addr,
                                                  void *destination,
                                                  uint16_t num)
{
  if (destination == NULL) {
    return HAL_ERROR;
  }

  //超出有效地址范围
  if ((addr < FLASH_ADDR_BASE) ||
      (addr >= (FLASH_ADDR_BASE + FLASH_PAGE_NUM * FLASH_PAGE_SIZE))) {
    return HAL_ERROR;
  }

  uint16_t *data = (uint16_t *)destination;

  /* Unlock the Program memory */
  HAL_FLASH_Unlock();

  for (uint16_t i = 0; i < num; i++) {
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, data[i]);
    addr += 2;  // address pointer move two step
  }

  /* Unlock the Program memory */
  HAL_FLASH_Lock();

  return HAL_OK;
}

// @function 往flash写入多个板半字，需要执行擦除检查；
// @para addr - 写入首地址；
// @para destination - 要写入的数据目标存放地址；
// @para num - 写入的半字数
uint16_t flash_buffer[FLASH_PAGE_SIZE / 2];
HAL_StatusTypeDef bsp_flash_write_n_hword_check(uint32_t addr, void *destination,
                                            uint16_t num)
{
  //超出有效地址范围
  if ((addr < FLASH_ADDR_BASE) ||
      (addr >= (FLASH_ADDR_BASE + FLASH_PAGE_NUM * FLASH_PAGE_SIZE))) {
    return HAL_ERROR;
  }

  if (destination == NULL) {
    return HAL_ERROR;
  }

  uint16_t *data = (uint16_t *)destination;
  uint16_t wait_write_num = num;

  uint16_t i;
  HAL_StatusTypeDef status = HAL_OK;

  HAL_FLASH_Unlock();

  uint32_t addr_offset = addr - FLASH_ADDR_BASE;
  uint16_t paper_postion = addr_offset / FLASH_PAGE_SIZE;
  uint16_t paper_offset = (addr_offset % FLASH_PAGE_SIZE) / 2;
  uint16_t paper_remain = FLASH_PAGE_SIZE / 2 - paper_offset;

  //如果剩余空间可完全容纳待写入的字节数
  if (paper_remain >= wait_write_num) {
    paper_remain = wait_write_num;
  }

  while (1) {
    //读取写入地址所在数据页
    bsp_flash_read_n_hword(FLASH_ADDR_BASE + paper_postion * FLASH_PAGE_SIZE,
                           (uint16_t *)flash_buffer,
                           FLASH_PAGE_SIZE / 2);

    for (i = 0; i < paper_remain; i++) {
      if (flash_buffer[paper_offset + i] != 0xffff) break;  //需要擦除
    }

    if (i < paper_remain) {
      // 擦除当前页
      bsp_flash_erase_npage(FLASH_ADDR_BASE + paper_postion * FLASH_PAGE_SIZE,
                          1);
      // 复制待写入数据到缓存
      for (i = 0; i < paper_remain; i++) {
        flash_buffer[paper_offset + i] = data[i];
      }

      //写入整页
      status = bsp_flash_write_n_hword_nocheck(
          FLASH_ADDR_BASE + paper_postion * FLASH_PAGE_SIZE,
          (uint16_t *)flash_buffer, FLASH_PAGE_SIZE / 2);
    } else {
      status =
          bsp_flash_write_n_hword_nocheck(addr, (uint16_t *)data, paper_remain);
    }

    if (status != HAL_OK) {
      return status;
    }

    if (paper_remain == wait_write_num) {
      break;
    } else {
      paper_postion += 1;
      paper_offset = 0;
      data += paper_remain;
      addr += paper_remain;
      wait_write_num -= paper_remain;

      if (wait_write_num > (FLASH_PAGE_SIZE / 2)) {
        paper_remain = FLASH_PAGE_SIZE / 2;
      } else {
        paper_remain = wait_write_num;
      }
    }
  }

  HAL_FLASH_Lock();

  return status;
}

// @function 擦除某一块区域的flash；
// @para start_page - 起始页的地址；
// @para end_page - 结束页的地址；
HAL_StatusTypeDef bsp_flash_erase_npage(uint32_t start_page, uint32_t n_page)
{
  HAL_FLASH_Unlock();  // unlock flash

  FLASH_EraseInitTypeDef EraseInitStruct;
  uint32_t PageError = 0;
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = start_page;
  EraseInitStruct.NbPages = n_page;
  HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);

  HAL_FLASH_Lock();  // lock

  return status;
}
