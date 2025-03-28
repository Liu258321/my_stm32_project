#include "stm32f4xx.h"
#include "task_test.h"
#include "my_delaytime.h"
#include "mpu6050__.h"
#include "uart.h"
#include <stdio.h>
#include "inv_mpu.h"
#include "watchdog.h"
#include "ssd1306.h"
#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define QUEUE_LEN       5

static mpu6050_data data = {0};

static void Task_MPU6050(void *src)
{
    watchdog_reload();
    printf("Task_MPU6050\n");
    while(1)
    {
        watchdog_reload();
        mpu6050_getdata(&data);
        // printf("%.1f %.1f\n",data.pitch,data.roll);
        vTaskDelay(1);
    }
    // vTaskDelete(NULL);
}

static void Task_SSD1306(void *src)
{
    printf("Task_SSD1306\n");
    vTaskDelay(100);
    vTaskSuspendAll();
    oled_tar_display(56,24,16,16,center_point,sizeof(center_point));
    xTaskResumeAll();
    // display_circle(50,20);
    // display_reverse(1);
    while(1) {
        vTaskSuspendAll();
        
        // display_num(abs(data.pitch),50,2);
        // display_num(abs(data.roll),90,2);
        // printf("%.1f %.1f\n",data.pitch,data.roll);
        display_circle(data.pitch,data.roll);

        xTaskResumeAll();
    }
    // vTaskDelete(NULL);
}


void All_Init(void *src)
{
    watchdog_reload();
    Delay_TIM6_Init();

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

    TaskHandle_t task_mpu6050=0;
    xTaskCreate(Task_MPU6050,"Task_MPU6050",2048,NULL,3,&task_mpu6050);

    TaskHandle_t task_ssd1306=0;
    xTaskCreate(Task_SSD1306,"Task_SSD1306",2048,NULL,2,&task_ssd1306);

    vTaskDelete(NULL);
}
