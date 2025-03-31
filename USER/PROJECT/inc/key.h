#ifndef KEY_H
#define KEY_H
#include "stm32f4xx.h"

extern volatile u8 key_ispress;

void Key_Interrupt_Init(void);
void EXTI0_IRQHandler(void);

#endif
