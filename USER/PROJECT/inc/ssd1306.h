#ifndef SSD1306_H
#define SSD1306_H
#include "stm32f4xx.h"
#include "oled_word.h"

void oled_display(void);
void oled_tar_display_page(u8 x,u8 y,u8 width,u8 high,u8 *pic);
void oled_tar_display(u8 x,u8 y,u8 width,u8 high,u8 *pic,u16 piclen);
void oled_clear(u8 black);
void ssd1306_init(void);
void display_num(u32 num,u8 x,u8 y);
void display_reverse(u8 flag);
void oled_center_point(void);
void display_circle(float pitch,float roll);

#endif
