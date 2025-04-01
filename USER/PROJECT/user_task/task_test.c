#include "stm32f4xx.h"
#include "task_test.h"
#include "my_delaytime.h"
#include "mpu6050__.h"
#include "uart.h"
#include <stdio.h>
#include "inv_mpu.h"
#include "watchdog.h"
#include "ssd1306.h"
#include "key.h"
#include "hc05.h"
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define QUEUE_LEN       5

static mpu6050_data data = {0};
volatile u8 key_ispress = 0;

static void Task_MPU6050(void *src)
{
    watchdog_reload();
    printf("Task_MPU6050\n");
    while(1)
    {
        watchdog_reload();
        NVIC_DisableIRQ(EXTI0_IRQn);
        mpu6050_getdata(&data);
        NVIC_EnableIRQ(EXTI0_IRQn);

        // printf("%.1f %.1f\n",data.pitch,data.roll);
        vTaskDelay(1);
    }
    // vTaskDelete(NULL);
}

static void Task_SSD1306(void *src)
{
    printf("Task_SSD1306\n");
    vTaskDelay(100);
    while(1) {
        vTaskSuspendAll();

        // taskENTER_CRITICAL();
        if(!key_ispress) {
            // printf("%.1f %.1f\n",data.pitch,data.roll);
            display_num(abs(data.pitch),50,2);
            display_num(abs(data.roll),90,2);
            oled_display();
        }else {
            display_circle(data.pitch,data.roll);
        }
        // taskEXIT_CRITICAL();

        xTaskResumeAll();
    }
    // vTaskDelete(NULL);
}

static void Task_HC05(void *src)
{
    char hc05_data[20] = {0};
    printf("Task_HC05\n");
    while(1) {
        vTaskSuspendAll();
        static u8 i=0;
        snprintf(hc05_data,sizeof(hc05_data),"%.1f %.1f\n",data.pitch,data.roll);
        for(i=0;i<strlen(hc05_data);i++) {
            USART_SendData(USART2,hc05_data[i]);
            while(USART_GetFlagStatus(USART2, USART_FLAG_TXE)!=SET);//等待发送完成
        }
        xTaskResumeAll();
    }
}


void All_Init(void *src)
{
    watchdog_reload();
    Delay_TIM6_Init();

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    // hc05_init();
    // printf("hc05_init\n");

    uint16_t count=0;
    printf("dmp initing\n");
    while(mpu_dmp_init())
    {
        watchdog_reload();
        vTaskDelay(1);
        count++;
        if(count>10)
        {
            printf("\ndmp init error\n");
            break;
        }
        printf("reinit\n");
    }
    printf("\ndmp init success\n");
    watchdog_reload();
    ssd1306_init();
    printf("ssd1306_init success\n");
    Key_Interrupt_Init();

    TaskHandle_t task_mpu6050=0;
    xTaskCreate(Task_MPU6050,"Task_MPU6050",2048,NULL,3,&task_mpu6050);

    TaskHandle_t task_ssd1306=0;
    xTaskCreate(Task_SSD1306,"Task_SSD1306",2048,NULL,2,&task_ssd1306);

    // TaskHandle_t task_hc05=0;
    // xTaskCreate(Task_HC05,"Task_HC05",2048,NULL,2,&task_hc05);

    vTaskDelete(NULL);
}
