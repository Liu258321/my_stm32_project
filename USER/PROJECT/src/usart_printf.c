#include "usart_printf.h"
#include "stm32f4xx.h"

int fputc(int c,FILE* stram)
{
    USART_SendData(USART1,c&0xff);
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)!=SET);
    return 0;
}
