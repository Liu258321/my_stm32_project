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
   
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//ʹ��GPIOBʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2,ENABLE);//ʹ��I2C2ʱ��

    I2C_DeInit(I2C2);

    GPIO_InitTypeDef  GPIO_InitStructure={0};
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;//��©���
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
    GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��

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
    //��ʼ�ź�
    I2C_GenerateSTART(I2C2,ENABLE);
    // printf("MPU_Write_Len %x %x %x\n",I2C2->CR1,I2C2->SR1,I2C2->SR2);
   

    //���źŷ��ͳɹ�
    while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_MODE_SELECT)!= SUCCESS ){
        // printf("%x %x\n",I2C2->SR1,I2C2->SR2);
    }
   

    //���ʹӵ�ַ
    I2C_Send7bitAddress(I2C2,addr<<1,I2C_Direction_Transmitter);
   
    // printf("%x %x\n",I2C2->SR1,I2C2->SR2);
   
    //���źŷ��ͳɹ�
    while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) != SUCCESS);
   
   

    //��������
    I2C_SendData(I2C2,reg);
   
   
    //���źŷ��ͳɹ�
    while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_BYTE_TRANSMITTING) != SUCCESS);
   
   
    for(u8 i=0;i<len;i++) {
        //��������
        I2C_SendData(I2C2,*(buf+i));
       
       
        //���źŷ��ͳɹ�
        while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_BYTE_TRANSMITTING) != SUCCESS);
       
    }

    //����ֹͣ�ź�
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

    //��ʼ�ź�
    I2C_GenerateSTART(I2C2,ENABLE);
   
    //���źŷ��ͳɹ�
    while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_MODE_SELECT) != SUCCESS);
    // printf("%d\n",__LINE__);

    //���ʹӵ�ַ
    I2C_Send7bitAddress(I2C2,addr<<1,I2C_Direction_Transmitter);
    //���źŷ��ͳɹ�
    while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) != SUCCESS);
   

    //���ͼĴ�����ַ
    I2C_SendData(I2C2,reg);
   
    //���źŷ��ͳɹ�
    while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_BYTE_TRANSMITTING) != SUCCESS);
   

    //��ʼ�ź�
    I2C_GenerateSTART(I2C2,ENABLE);
   
    //���źŷ��ͳɹ�
    while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_MODE_SELECT) != SUCCESS);
   
   
    //���ʹӵ�ַ
    I2C_Send7bitAddress(I2C2,addr<<1,I2C_Direction_Receiver);
   
    //���źŷ��ͳɹ�
    while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) != SUCCESS);
    // printf("%d\n",__LINE__);
   
   
    while(len--)
	{
		//�Ƿ������һ���ֽڣ��������ͷ�Ӧ���ź�
		if( len==0)
        {
            //���ͷ�Ӧ���ź�
            I2C_AcknowledgeConfig(I2C2,DISABLE);
            //����ֹͣ�ź�
            I2C_GenerateSTOP(I2C2,ENABLE);
        }

        //���EV7�¼�
        while(I2C_CheckEvent(I2C2,I2C_EVENT_MASTER_BYTE_RECEIVED) != SUCCESS);
	 
        *buf=I2C_ReceiveData(I2C2);
        buf++; 
	 
	}
	
	//���¿���Ӧ���ź�
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
