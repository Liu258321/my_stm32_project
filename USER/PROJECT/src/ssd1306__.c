#include "FreeRTOS.h"
#include "task.h"
#include "ssd1306.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//commond
#define SSD1306_SLAVE_ADDR      0x3c
#define SSD1306_TRANSFER_CMD    0x00
#define SSD1306_TRANSFER_DATA   0x40

static u8 oled_buffer[8][128] = {0};

static void ssd1306_i2c_init(void)
{
    // printf("%x %x %x\n",I2C1->CR1,I2C1->SR1,I2C1->SR2);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//ʹ��GPIOBʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);//ʹ��I2C1ʱ��
    // printf("%x %x %x\n",I2C1->CR1,I2C1->SR1,I2C1->SR2);

    I2C_DeInit(I2C1);

    GPIO_InitTypeDef  GPIO_InitStructure={0};
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;//��©���
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
    GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��

    GPIO_PinAFConfig(GPIOB,GPIO_PinSource6,GPIO_AF_I2C1);
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource7,GPIO_AF_I2C1);


    I2C_InitTypeDef I2C_InitTypeDef={0};
    I2C_InitTypeDef.I2C_Ack         =   I2C_Ack_Enable;
    I2C_InitTypeDef.I2C_AcknowledgedAddress =   I2C_AcknowledgedAddress_7bit;
    I2C_InitTypeDef.I2C_ClockSpeed  =   400000;
    I2C_InitTypeDef.I2C_DutyCycle   =   I2C_DutyCycle_2;
    I2C_InitTypeDef.I2C_Mode        =   I2C_Mode_I2C;
    I2C_InitTypeDef.I2C_OwnAddress1 =   0x1;

    // printf("%x %x %x\n",I2C1->CR1,I2C1->SR1,I2C1->SR2);

    I2C_Init(I2C1,&I2C_InitTypeDef);
    I2C_Cmd(I2C1,ENABLE);
    // printf("%x %x %x\n",I2C1->CR1,I2C1->SR1,I2C1->SR2);
}

static void i2c_send(const u8 data,const u8 cmd)
{
    // printf("i2c_send %x %x %x\n",I2C1->CR1,I2C1->SR1,I2C1->SR2);
    //��ʼ�ź�
    I2C_GenerateSTART(I2C1,ENABLE);
   
    //���źŷ��ͳɹ�
    while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_MODE_SELECT)!= SUCCESS );
    //printf("%d\n",__LINE__);

    //���ʹӵ�ַ
    I2C_Send7bitAddress(I2C1,SSD1306_SLAVE_ADDR<<1,I2C_Direction_Transmitter);
   
    //���źŷ��ͳɹ�
    while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) != SUCCESS);
    //printf("%d\n",__LINE__);

    //��������
    I2C_SendData(I2C1,cmd);
   
    //���źŷ��ͳɹ�
    while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTING) != SUCCESS);
    //printf("%d\n",__LINE__);
   
    //��������
    I2C_SendData(I2C1,data);
    
    //���źŷ��ͳɹ�
    while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTING) != SUCCESS);
    //printf("%d\n",__LINE__);

    //����ֹͣ�ź�
    I2C_GenerateSTOP(I2C1,ENABLE);
    // printf("send success\n");
}

void oled_display(void) 
{
    NVIC_DisableIRQ(EXTI0_IRQn);
    static u8 i,j;
    //��ʼ�ź�
    I2C_GenerateSTART(I2C1,ENABLE);
   
    //���źŷ��ͳɹ�
    while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_MODE_SELECT)!= SUCCESS );
    //printf("%d\n",__LINE__);

    //���ʹӵ�ַ
    I2C_Send7bitAddress(I2C1,SSD1306_SLAVE_ADDR<<1,I2C_Direction_Transmitter);
   
    //���źŷ��ͳɹ�
    while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) != SUCCESS);
    //printf("%d\n",__LINE__);

    //��������
    I2C_SendData(I2C1,SSD1306_TRANSFER_DATA);
   
    //���źŷ��ͳɹ�
    while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTING) != SUCCESS);
    //printf("%d\n",__LINE__);
   
    //��������
    for(i=0;i<8;i++) {
        for(j=0;j<128;j++) {
            I2C_SendData(I2C1,oled_buffer[i][j]);
            //���źŷ��ͳɹ�
            while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTING) != SUCCESS);
        }
    }
    // printf("%d\n",__LINE__);
    // ����ֹͣ�ź�
    I2C_GenerateSTOP(I2C1,ENABLE);
    NVIC_EnableIRQ(EXTI0_IRQn);
}

void oled_clear(u8 black)
{
    if(black) memset(oled_buffer,0,sizeof(oled_buffer));
    else memset(oled_buffer,-1,sizeof(oled_buffer));
    oled_display();
}

void ssd1306_init(void)
{
    ssd1306_i2c_init();
    vTaskDelay(100);
    // printf("iic init success\n");

    //�ر���Ļ��ʾ
    i2c_send(0xAE,SSD1306_TRANSFER_CMD);

    // //������ʼ�͵�ַ
    // i2c_send(0x00,SSD1306_TRANSFER_CMD);

    // //������ʼ�ߵ�ַ
    // i2c_send(0x10,SSD1306_TRANSFER_CMD);

    //������ʾ��ʼ��
    // i2c_send(0x40,SSD1306_TRANSFER_CMD);

    //����ҳ��Ѱַģʽ
    // i2c_send(0xB0,SSD1306_TRANSFER_CMD);

    //���öԱȶ�
    i2c_send(0x81,SSD1306_TRANSFER_CMD);
    i2c_send(0xFF,SSD1306_TRANSFER_CMD);

    //������ӳ��
    i2c_send(0xA1,SSD1306_TRANSFER_CMD);

    //������ʾ
    i2c_send(0xA6,SSD1306_TRANSFER_CMD);

    //���ö��ر���
    i2c_send(0xA8,SSD1306_TRANSFER_CMD);
    i2c_send(0x3F,SSD1306_TRANSFER_CMD);

    //����ɨ���������
    i2c_send(0xC8,SSD1306_TRANSFER_CMD);

    //������ʾƫ��
    // i2c_send(0xD3,SSD1306_TRANSFER_CMD);
    // i2c_send(0x00,SSD1306_TRANSFER_CMD);

    //���÷�Ƶ��
    i2c_send(0xD5,SSD1306_TRANSFER_CMD);
    i2c_send(0x80,SSD1306_TRANSFER_CMD);

    
    // i2c_send(0xD8,SSD1306_TRANSFER_CMD);
    // i2c_send(0x05,SSD1306_TRANSFER_CMD);

    //����Ԥ�������
    i2c_send(0xD9,SSD1306_TRANSFER_CMD);
    i2c_send(0xF1,SSD1306_TRANSFER_CMD);

    //����COM��������
    i2c_send(0xDA,SSD1306_TRANSFER_CMD);
    i2c_send(0x12,SSD1306_TRANSFER_CMD);

    //����VCOM
    i2c_send(0xDB,SSD1306_TRANSFER_CMD);
    i2c_send(0x30,SSD1306_TRANSFER_CMD);

    //������ɱ�
    i2c_send(0x8D,SSD1306_TRANSFER_CMD);
    i2c_send(0x14,SSD1306_TRANSFER_CMD);

    //�ر�������ʾ
    i2c_send(0xA4,SSD1306_TRANSFER_CMD);

    //ͣ�ù�����
    i2c_send(0x2E,SSD1306_TRANSFER_CMD);


    //�����ڴ�Ѱַģʽ
    i2c_send(0x20,SSD1306_TRANSFER_CMD);
    i2c_send(0x00,SSD1306_TRANSFER_CMD);        //ˮƽѰַ
    // i2c_send(0x02,SSD1306_TRANSFER_CMD);     //ҳѰַ

    //�����е�ַ��Χ
    i2c_send(0x21,SSD1306_TRANSFER_CMD);
    i2c_send(0x00,SSD1306_TRANSFER_CMD);
    i2c_send(0x7F,SSD1306_TRANSFER_CMD);

    //����ҳ��ַ��Χ
    i2c_send(0x22,SSD1306_TRANSFER_CMD);
    i2c_send(0x00,SSD1306_TRANSFER_CMD);
    i2c_send(0x07,SSD1306_TRANSFER_CMD);

    //����Ļ��ʾ
    i2c_send(0xAF,SSD1306_TRANSFER_CMD);

    // while(1);

    oled_clear(0);
    vTaskDelay(1000);
    oled_clear(1);
}

// static void oled_part_reflash(u8 x,u8 y_page,u8 width,u8 high) 
// {
//     for(u8 i=0;i<high;i++) {
//         i2c_send(0xB0+y_page+i,SSD1306_TRANSFER_CMD);
//         i2c_send(0x0F&x,SSD1306_TRANSFER_CMD);
//         i2c_send(0x10|((x&0xF0)>>4),SSD1306_TRANSFER_CMD);
//         for(u8 j=0;j<width;j++) {
//             i2c_send(oled_buffer[i+y_page][j+x],SSD1306_TRANSFER_DATA);
//         }
//     }
// }

void oled_tar_display_page(u8 x,u8 y,u8 width,u8 high,u8 *pic)
{
    u8 piclen=0;
    for(u8 i=0;i < high;i++) {
        for(u8 j=0;j<width;j++) {
            if(pic == NULL) {
                oled_buffer[y+i][x+j] = 0;
                continue;
            }
            oled_buffer[y+i][x+j] = pic[piclen++];
        }
    }
}

void oled_tar_display(u8 x,u8 y,u8 width,u8 high,u8 *pic,u16 piclen)
{
    static u8 offset,pagestart_y,pageend_y,ii,i;
    offset = y % 8;
    pageend_y = (y + high) / 8;      
    y /= 8;
    pagestart_y = y;     
    ii = 0;

    if(pic == NULL) {
        while(pagestart_y <= pageend_y) {
            for(i = 0;i < width;i++,ii++) {
                if(ii < width) {                    //����ֻ��ƫ��offset
                    oled_buffer[pagestart_y][x+i] &= ~(0xff << offset);
                }else if(ii < piclen){              //�м䲿��
                    oled_buffer[pagestart_y][x+i] = 0;
                }else {                             //ĩ��ƫ�Ƴ�������
                    oled_buffer[pagestart_y][x+i] &= ~(0xff >> (8 - offset));
                }
            }
            pagestart_y++;
        }
    }else {
        while(pagestart_y <= pageend_y) {
            for(i = 0;i < width;i++,ii++) {
                if(ii < width) {                    //����ֻ��ƫ��offset
                    oled_buffer[pagestart_y][x+i] = pic[ii] << offset;
                }else if(ii < piclen){              //�ⲿ����Ҫ�����б��˳������ֽ��
                    oled_buffer[pagestart_y][x+i] = (pic[ii-width] >> (8 - offset)) | (pic[ii] << offset);
                }else {                             //ĩ��ƫ�Ƴ�������
                    oled_buffer[pagestart_y][x+i] = pic[ii-width] >> (8 - offset);
                }
            }
            pagestart_y++;
        }
    }
}

void display_num(u32 num,u8 x,u8 y)
{
    oled_tar_display(x,y,24,16,NULL,48);
    if(num==0) {
        oled_tar_display(x,y,8,16,number_word[num],sizeof(number_word[num]));
        return;
    }

    static u8 dig = 0;
    static u16 pow_ = 0;
    dig = log10(abs(num)) + 1;
    pow_ = pow(10,dig-1);
    for( ;dig>0;dig--) {
        oled_tar_display(x,y,8,16,number_word[num/pow_],sizeof(number_word[num/pow_]));

        num %= pow_;
        pow_ /= 10;
        x += 8;
    }

}

void display_reverse(u8 flag) 
{
    if (flag) {
        i2c_send(0xA7,SSD1306_TRANSFER_CMD);;  // ��ɫ��ʾ
    } else {
        i2c_send(0xA6,SSD1306_TRANSFER_CMD);;  // ������ʾ
    }
}

#define CIRCLE_CENTER_X         56
#define CIRCLE_CENTER_Y         24
#define CIRCLE_LIMIT_X_START    40  //56-16
#define CIRCLE_LIMIT_X_END      72  //56+16
#define CIRCLE_LIMIT_Y_START    8   //24-16
#define CIRCLE_LIMIT_Y_END      40  //24+16
#define CIRCLE_WIDTH            16
#define CIRCLE_HIGH_PAGE        2   //16/8

#define MAX(x,y)                ((x) > (y) ?(x) : (y))


static u8 ball_is_center(u8 x,u8 y)
{   
    return (x >= CIRCLE_LIMIT_X_START && x <= CIRCLE_LIMIT_X_END) && (y >= CIRCLE_LIMIT_Y_START && y <= CIRCLE_LIMIT_Y_END);
}

static void oled_tar_write(u8 x,u8 y,u8 width,u8 high,u8 *pic,u16 piclen)
{
    static u8 offset,pagestart_y,pageend_y,ii,i;
    offset = y % 8;
    pageend_y = (y + high) / 8;    
    y /= 8;
    pagestart_y = y;
    ii = 0;

    while(pagestart_y <= pageend_y) {
        for(i = 0;i < width;i++,ii++) {
            if(ii < width) {
                oled_buffer[pagestart_y][x+i] ^= pic[ii] << offset;
            }else if(ii < piclen){
                oled_buffer[pagestart_y][x+i] ^= (pic[ii-width] >> (8 - offset)) | (pic[ii] << offset);
            }else {
                oled_buffer[pagestart_y][x+i] ^= pic[ii-width] >> (8 - offset);
            }
        }
        pagestart_y++;
    }

    // oled_part_reflash(x,y,width,pageend_y - y +1);
}

void oled_center_point(void)
{
    static u8 i = 0;
    for(i=0;i<16;i++) {
        oled_buffer[3][56+i] |= 0x80;
        oled_buffer[4][56+i] |= 0x01;
    }
    oled_buffer[3][63] = 0xff;
    oled_buffer[3][64] = 0xff;
    oled_buffer[4][63] = 0xff;
    oled_buffer[4][64] = 0xff;
    // oled_part_reflash(56,3,16,2);
}

void display_circle(float pitch,float roll)
{
    static u8 last_x = 0,last_y = 0;
    static u8 cur_x = 0,cur_y = 0;

    //����ƫ��
    cur_x = CIRCLE_CENTER_X - roll  / 180.0f * 56.0f;
    cur_y = CIRCLE_CENTER_Y + pitch / 180.0f * 24.0f;

    //�����ظ����µ�����������
    if((cur_x == last_x) && (cur_y == last_y)) {
        return;
    }
    //������λ��
    oled_tar_display(last_x,last_y,CIRCLE_WIDTH,CIRCLE_WIDTH,NULL,sizeof(circle_pic));

    oled_center_point();

    //������ġ�ʮ����ײ
    if(ball_is_center(cur_x,cur_y)) {
        //��ʾ��λ��
        oled_tar_write(cur_x,cur_y,CIRCLE_WIDTH,CIRCLE_WIDTH,circle_pic,sizeof(circle_pic));
    }else {
        //��ʾ��λ��
        oled_tar_display(cur_x,cur_y,CIRCLE_WIDTH,CIRCLE_WIDTH,circle_pic,sizeof(circle_pic));
    }

    // oled_part_reflash(last_x,last_y/8,16,3);
    // oled_part_reflash(56,3,16,2);
    // oled_part_reflash(cur_x,cur_y/8,16,3);
    oled_display();
    //���¾�λ��
    last_x = cur_x;
    last_y = cur_y;

    if(cur_x == CIRCLE_CENTER_X && cur_y == CIRCLE_CENTER_Y) {
        display_reverse(1);
    } else {
        display_reverse(0);
    }
}
