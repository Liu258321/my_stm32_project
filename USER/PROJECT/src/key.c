#include "key.h"
#include <stdio.h>
#include "ssd1306.h"

void Key_Interrupt_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    // 1. 开启时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); // GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE); // SYSCFG时钟（STM32F4系列需要）

    // 2. 配置GPIO
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;        // 输入模式
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;        // 上拉电阻
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;  // 高速模式
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 3. 映射中断线
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0); // PA0映射到EXTI0[6](@ref)

    // 4. 配置EXTI
    EXTI_InitStruct.EXTI_Line = EXTI_Line0;          // 中断线0
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt; // 中断模式
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising; // 上升沿触发
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;           // 使能中断线
    EXTI_Init(&EXTI_InitStruct);

    // 5. 配置NVIC
    NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;     // 中断通道
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2; // 抢占优先级
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;        // 子优先级
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;      // 使能中断
    NVIC_Init(&NVIC_InitStruct);
}

// 中断服务函数
void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) 
    {
        printf("press\n");
        key_ispress = !key_ispress;            // 设置按键标志
        oled_clear(1);
        EXTI_ClearITPendingBit(EXTI_Line0); // 清除中断标志[3,7](@ref)
    }
}
