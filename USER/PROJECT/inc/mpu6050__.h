#ifndef MPU6050_H
#define MPU6050_H

#include "stm32f4xx.h"

typedef struct {
    float pitch;
    float roll;
    float yaw;
    float x_acc;
    float y_acc;
    float z_acc;
}mpu6050_data;

void IIC_Init(void);
u8 MPU_Write_Len(u8 addr,u8 reg,u8 len,u8 *buf);
u8 MPU_Read_Len(u8 addr,u8 reg,u8 len,u8 *buf);
void mpu6050_getdata(mpu6050_data *data);

#endif
