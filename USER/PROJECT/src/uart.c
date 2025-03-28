#include "stm32f4xx.h"

/*初始化串口1*/
void My_Uart1Init(uint32_t USART_BaudRate)
{
	/*1.配置GPIO口*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct={0};
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;//复用
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1);
	/*2.配置USART1控制器*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	USART_InitTypeDef USART_InitStruct={0};
	USART_InitStruct.USART_BaudRate = USART_BaudRate;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无流控
	USART_InitStruct.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;//全双工
	USART_InitStruct.USART_Parity = USART_Parity_No;//无校验
	USART_InitStruct.USART_StopBits = USART_StopBits_1;//1位停止位
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;//8位数据位
	USART_Init(USART1, &USART_InitStruct);	
	/*3.配置中断*/
	// USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	// NVIC_InitTypeDef NVIC_InitStruct={0};
	// NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
	// NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	// NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
	// NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
	// NVIC_Init(&NVIC_InitStruct);
	/*4.使能串口*/
	USART_Cmd(USART1,ENABLE);
}

/*
	UART1_Send_Datas:串口发送数据
	@sendbuf:指向需要发送的数据
	@len:需要发送的数据的长度
	返回值为空
*/
void UART1_Send_Datas(u8* sendbuf,u16 len)
{
	int i;
	for(i=0;i<len;i++)
	{
		USART_SendData(USART1, sendbuf[i]);
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)!=SET);//等待发送完成
	}
}

uint16_t USART1datas;
void USART1_IRQHandler(void)
{
    if(USART_GetITStatus(USART1,USART_IT_RXNE)==SET)
    {
        //读取数据
        USART1datas = USART_ReceiveData(USART1);
        USART_ClearITPendingBit(USART1,USART_IT_RXNE);     
	}
}



