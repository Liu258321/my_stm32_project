#include "stm32f4xx.h"
#include "my_delaytime.h"

/* 
    * @brief    init tim6 to count 1 us
    * @param    none
    * @retval   none
 */
void Delay_TIM6_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE);

    TIM_TimeBaseInitTypeDef tim6_init={0};
    tim6_init.TIM_Prescaler         =   45-1;
    tim6_init.TIM_CounterMode       =   TIM_CounterMode_Up;
    tim6_init.TIM_Period            =   2-1;
    tim6_init.TIM_ClockDivision     =   0;
    tim6_init.TIM_RepetitionCounter =   0;
    TIM_TimeBaseInit(TIM6,&tim6_init);

    TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE);

    NVIC_InitTypeDef tim6_nvic={0};
    tim6_nvic.NVIC_IRQChannel                   =   TIM6_DAC_IRQn;
    tim6_nvic.NVIC_IRQChannelPreemptionPriority =   2;
    tim6_nvic.NVIC_IRQChannelSubPriority        =   2;
    tim6_nvic.NVIC_IRQChannelCmd                =   ENABLE;
    NVIC_Init(&tim6_nvic);
}

static volatile int n=0;

void TIM6_DAC_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM6,TIM_IT_Update) == SET)
    {
        n--;
        TIM_ClearITPendingBit(TIM6,TIM_IT_Update);
    }
}

/* 
    * @brief    delay specified value ms,before use must init TIM6
    * @param    delay_us: specified value you want to delay (ms)
    * @retval   none
 */
void Delay_Us(int delay_)
{
    n=delay_;
    TIM_Cmd(TIM6,ENABLE);
    while(n>0);
    TIM_Cmd(TIM6,DISABLE);
}
