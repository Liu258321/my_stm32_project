#ifndef UART_H
#define UART_H

#include "stm32f4xx.h"

void My_Uart1Init(uint32_t USART_BaudRate);
void UART1_Send_Datas(u8* sendbuf,u16 len);


#endif 

