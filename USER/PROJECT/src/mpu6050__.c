#include "FreeRTOS.h"
#include "task.h"
#include "mpu6050__.h"
#include "my_delaytime.h"
#include "inv_mpu.h"
#include <stdio.h>

#define COLECT_TIME     20

void IIC_Init(void)
{
    // printf("IIC_Init \n");
   
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIOB时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2,ENABLE);//使能I2C2时钟

    I2C_DeInit(I2C2);

    GPIO_InitTypeDef  GPIO_InitStructure={0};
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;//开漏输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化

    GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_I2C2);
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_I2C2);


    I2C_InitTypeDef I2C_InitTypeDef={0};
    I2C_InitTypeDef.I2C_Ack         =   I2C_Ack_Enable;
    I2C_InitTypeDef.I2C_AcknowledgedAddress =   I2C_AcknowledgedAddress_7bit;
    I2C_InitTypeDef.I2C_ClockSpeed  =   400000;
    I2C_InitTypeDef.I2C_DutyCycle   =   I2C_DutyCycle_2;
    I2C_InitTypeDef.I2C_Mode        =   I2C_Mode_I2C;
    I2C_InitTypeDef.I2C_OwnAddress1 =   0x1;

    I2C_Init(I2C2,&I2C_InitTypeDef);
    I2C_Cmd(I2C2,ENABLE);

    // printf("iic init success\n");

    // printf("%x %x %x\n",I2C2->CR1,I2C2->SR1,I2C2->SR2);
}

u8 MPU_Write_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{
    // printf("sending %d\n",len);
    //起始信号
    I2C_GenerateSTART(I2C2,ENABLE);
    // printf("MPU_Write_Len %x %x %x\n",I2C2->CR1,I2C2->SR1,I2C2->SR2);
   

    //等信号发送成功
    while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_MODE_SELECT)!= SUCCESS ){
        // printf("%x %x\n",I2C2->SR1,I2C2->SR2);
    }
   

    //发送从地址
    I2C_Send7bitAddress(I2C2,addr<<1,I2C_Direction_Transmitter);
   
    // printf("%x %x\n",I2C2->SR1,I2C2->SR2);
   
    //等信号发送成功
    while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) != SUCCESS);
   
   

    //发送数据
    I2C_SendData(I2C2,reg);
   
   
    //等信号发送成功
    while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_BYTE_TRANSMITTING) != SUCCESS);
   
   
    for(u8 i=0;i<len;i++) {
        //发送数据
        I2C_SendData(I2C2,*(buf+i));
       
       
        //等信号发送成功
        while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_BYTE_TRANSMITTING) != SUCCESS);
       
    }

    //发送停止信号
    I2C_GenerateSTOP(I2C2,ENABLE);
    // printf("send success\n");
    // printf("send success %x\n",I2C2->SR2);
    return 0;  
}

u8 MPU_Read_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{
    // vTaskDelay(5);
    // printf("reading %d %x\n",len,I2C2->SR2);
    // printf("reading %d\n",len);
    while(I2C_GetFlagStatus(I2C2,I2C_FLAG_BUSY)==SET);

    //起始信号
    I2C_GenerateSTART(I2C2,ENABLE);
   
    //等信号发送成功
    while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_MODE_SELECT) != SUCCESS);
    // printf("%d\n",__LINE__);

    //发送从地址
    I2C_Send7bitAddress(I2C2,addr<<1,I2C_Direction_Transmitter);
    //等信号发送成功
    while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) != SUCCESS);
   

    //发送寄存器地址
    I2C_SendData(I2C2,reg);
   
    //等信号发送成功
    while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_BYTE_TRANSMITTING) != SUCCESS);
   

    //起始信号
    I2C_GenerateSTART(I2C2,ENABLE);
   
    //等信号发送成功
    while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_MODE_SELECT) != SUCCESS);
   
   
    //发送从地址
    I2C_Send7bitAddress(I2C2,addr<<1,I2C_Direction_Receiver);
   
    //等信号发送成功
    while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) != SUCCESS);
    // printf("%d\n",__LINE__);
   
   
    while(len--)
	{
		//是否是最后一个字节，若是则发送非应答信号
		if( len==0)
        {
            //发送非应答信号
            I2C_AcknowledgeConfig(I2C2,DISABLE);
            //发送停止信号
            I2C_GenerateSTOP(I2C2,ENABLE);
        }

        //检测EV7事件
        while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_BYTE_RECEIVED) != SUCCESS);
	 
        *buf=I2C_ReceiveData(I2C2);
        buf++; 
	 
	}
	
	//重新开启应答信号
	I2C_AcknowledgeConfig(I2C2,ENABLE);

    // printf("read success\n");
    // printf("read success %x\n",I2C2->SR2);
    return 0;  
}

void mpu6050_getdata(mpu6050_data *data)
{
    static float yaw=0,xacc=0,yacc=0,zacc=0;
    while(mpu_dmp_get_data(&(data->pitch),&(data->roll),&yaw,&xacc,&yacc,&zacc));
}
