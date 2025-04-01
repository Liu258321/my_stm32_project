#include "stm32f4xx.h"
#include "hc05.h"
#include <stdio.h>

//pd5====USART2_TX
//pd6====USART2_RX

void hc05_init(void)
{
    /*1.配置GPIO口*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct={0};
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;//复用
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_6;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOD,&GPIO_InitStruct);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource5,GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource6,GPIO_AF_USART1);
	/*2.配置USART1控制器*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	USART_InitTypeDef USART_InitStruct={0};
	USART_InitStruct.USART_BaudRate = 9600;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无流控
	USART_InitStruct.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;//全双工
	USART_InitStruct.USART_Parity = USART_Parity_No;//无校验
	USART_InitStruct.USART_StopBits = USART_StopBits_1;//1位停止位
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;//8位数据位
	USART_Init(USART2, &USART_InitStruct);	
	/*3.配置中断*/
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
	NVIC_InitTypeDef NVIC_InitStruct={0};
	NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&NVIC_InitStruct);
	/*4.使能串口*/
	USART_Cmd(USART2,ENABLE);
}

u8 hc05_data = 0;
void USART2_IRQHandler(void)
{
    printf("receive:");
    if(USART_GetITStatus(USART2,USART_IT_RXNE)==SET)
    {
        //读取数据
        hc05_data = USART_ReceiveData(USART2);
        printf("%x - %c\n",hc05_data,hc05_data);
        USART_ClearITPendingBit(USART2,USART_IT_RXNE);     
	}
}
