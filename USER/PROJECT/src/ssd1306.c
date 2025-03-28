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

static void i2c_init(void)
{
    // printf("%x %x %x\n",I2C1->CR1,I2C1->SR1,I2C1->SR2);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIOB时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);//使能I2C1时钟
    // printf("%x %x %x\n",I2C1->CR1,I2C1->SR1,I2C1->SR2);

    GPIO_InitTypeDef  GPIO_InitStructure={0};
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;//开漏输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化

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

static void dma_send(void)
{
    //起始信号
    I2C_GenerateSTART(I2C1,ENABLE);
   
    //等信号发送成功
    while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_MODE_SELECT)!= SUCCESS );
    //printf("%d\n",__LINE__);

    //发送从地址
    I2C_Send7bitAddress(I2C1,SSD1306_SLAVE_ADDR<<1,I2C_Direction_Transmitter);
   
    //等信号发送成功
    while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) != SUCCESS);
    //printf("%d\n",__LINE__);

    //发送数据
    I2C_SendData(I2C1,SSD1306_TRANSFER_DATA);
   
    //等信号发送成功
    while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTING) != SUCCESS);
}


static void dma_init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);
    DMA_DeInit(DMA1_Stream0);

    DMA_InitTypeDef dma_config;
    dma_config.DMA_Channel              =   DMA_Channel_1;
    dma_config.DMA_PeripheralBaseAddr   =   (uint32_t)&(I2C1->DR);
    dma_config.DMA_Memory0BaseAddr      =   (uint32_t)oled_buffer;
    dma_config.DMA_DIR                  =   DMA_DIR_MemoryToPeripheral;
    dma_config.DMA_BufferSize           =   1024;
    dma_config.DMA_PeripheralInc        =   DMA_PeripheralInc_Disable;
    dma_config.DMA_MemoryInc            =   DMA_MemoryInc_Enable;
    dma_config.DMA_PeripheralDataSize   =   DMA_PeripheralDataSize_Byte;
    dma_config.DMA_MemoryDataSize       =   DMA_MemoryDataSize_Byte;
    dma_config.DMA_Mode                 =   DMA_Mode_Circular;
    // dma_config.DMA_Mode                 =   DMA_Mode_Normal;
    dma_config.DMA_Priority             =   DMA_Priority_Medium;
    dma_config.DMA_FIFOMode             =   DMA_FIFOMode_Disable;
    // dma_config.DMA_FIFOThreshold        =   DMA_FIFOThreshold_Full;
    // dma_config.DMA_MemoryBurst          =   DMA_MemoryBurst_Single;
    // dma_config.DMA_PeripheralBurst      =   DMA_PeripheralBurst_Single;

    DMA_Init(DMA1_Stream0,&dma_config);
    // DMA_ITConfig(DMA1_Stream7, DMA_IT_TC | DMA_IT_TE, ENABLE);
    dma_send();
    DMA_Cmd(DMA1_Stream0,ENABLE);
    I2C_DMACmd(I2C1,ENABLE);
}

static void i2c_send(const u8 data,const u8 cmd)
{
    // printf("i2c_send %x %x %x\n",I2C1->CR1,I2C1->SR1,I2C1->SR2);
    //起始信号
    I2C_GenerateSTART(I2C1,ENABLE);
   
    //等信号发送成功
    while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_MODE_SELECT)!= SUCCESS );
    //printf("%d\n",__LINE__);

    //发送从地址
    I2C_Send7bitAddress(I2C1,SSD1306_SLAVE_ADDR<<1,I2C_Direction_Transmitter);
   
    //等信号发送成功
    while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) != SUCCESS);
    //printf("%d\n",__LINE__);

    //发送数据
    I2C_SendData(I2C1,cmd);
   
    //等信号发送成功
    while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTING) != SUCCESS);
    //printf("%d\n",__LINE__);
   
    //发送数据
    I2C_SendData(I2C1,data);
    
    //等信号发送成功
    while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTING) != SUCCESS);
    //printf("%d\n",__LINE__);

    //发送停止信号
    I2C_GenerateSTOP(I2C1,ENABLE);
    // printf("send success\n");
}
static void oled_display(void) 
{
    u8 i,j;
    for(i=0;i<8;i++) {
        i2c_send(0xB0+i,SSD1306_TRANSFER_CMD);
        i2c_send(0x00,SSD1306_TRANSFER_CMD);
        i2c_send(0x10,SSD1306_TRANSFER_CMD);
        for(j=0;j<128;j++) {
            i2c_send(oled_buffer[i][j],SSD1306_TRANSFER_DATA);
        }
    }
}

void oled_clear(u8 black)
{
    if(black) memset(oled_buffer,0,sizeof(oled_buffer));
    else memset(oled_buffer,-1,sizeof(oled_buffer));
    oled_display();
}

void ssd1306_init(void)
{
    i2c_init();
    // printf("iic init success\n");

    //关闭屏幕显示
    i2c_send(0xAE,SSD1306_TRANSFER_CMD);

    // //设置起始低地址
    // i2c_send(0x00,SSD1306_TRANSFER_CMD);

    // //设置起始高地址
    // i2c_send(0x10,SSD1306_TRANSFER_CMD);

    //设置显示起始行
    // i2c_send(0x40,SSD1306_TRANSFER_CMD);

    //设置页面寻址模式
    // i2c_send(0xB0,SSD1306_TRANSFER_CMD);

    //设置对比度
    i2c_send(0x81,SSD1306_TRANSFER_CMD);
    i2c_send(0xFF,SSD1306_TRANSFER_CMD);

    //设置重映射
    i2c_send(0xA1,SSD1306_TRANSFER_CMD);

    //正常显示
    i2c_send(0xA6,SSD1306_TRANSFER_CMD);

    //设置多重比率
    i2c_send(0xA8,SSD1306_TRANSFER_CMD);
    i2c_send(0x3F,SSD1306_TRANSFER_CMD);

    //设置扫描输出方向
    i2c_send(0xC8,SSD1306_TRANSFER_CMD);

    //设置显示偏移
    // i2c_send(0xD3,SSD1306_TRANSFER_CMD);
    // i2c_send(0x00,SSD1306_TRANSFER_CMD);

    //设置分频比
    i2c_send(0xD5,SSD1306_TRANSFER_CMD);
    i2c_send(0x80,SSD1306_TRANSFER_CMD);

    
    // i2c_send(0xD8,SSD1306_TRANSFER_CMD);
    // i2c_send(0x05,SSD1306_TRANSFER_CMD);

    //设置预充电周期
    i2c_send(0xD9,SSD1306_TRANSFER_CMD);
    i2c_send(0xF1,SSD1306_TRANSFER_CMD);

    //设置COM引脚配置
    i2c_send(0xDA,SSD1306_TRANSFER_CMD);
    i2c_send(0x12,SSD1306_TRANSFER_CMD);

    //设置VCOM
    i2c_send(0xDB,SSD1306_TRANSFER_CMD);
    i2c_send(0x30,SSD1306_TRANSFER_CMD);

    //开启电荷泵
    i2c_send(0x8D,SSD1306_TRANSFER_CMD);
    i2c_send(0x14,SSD1306_TRANSFER_CMD);

    //关闭整个显示
    i2c_send(0xA4,SSD1306_TRANSFER_CMD);

    //停用滚动轴
    i2c_send(0x2E,SSD1306_TRANSFER_CMD);


    //设置内存寻址模式
    i2c_send(0x20,SSD1306_TRANSFER_CMD);
    // i2c_send(0x00,SSD1306_TRANSFER_CMD);
    i2c_send(0x02,SSD1306_TRANSFER_CMD);

    //设置列地址范围
    i2c_send(0x21,SSD1306_TRANSFER_CMD);
    i2c_send(0x00,SSD1306_TRANSFER_CMD);
    i2c_send(0x7F,SSD1306_TRANSFER_CMD);

    //设置页地址范围
    i2c_send(0x22,SSD1306_TRANSFER_CMD);
    i2c_send(0x00,SSD1306_TRANSFER_CMD);
    i2c_send(0x07,SSD1306_TRANSFER_CMD);

    //打开屏幕显示
    i2c_send(0xAF,SSD1306_TRANSFER_CMD);

    // dma_init();

    // // memset(oled_buffer,0,sizeof(oled_buffer));
    // vTaskDelay(3000);
    // printf("start\n");
    // memset(oled_buffer,-1,sizeof(oled_buffer));
    // printf("%x\n",oled_buffer[0][0]);

    // while(1);

    oled_clear(0);
    vTaskDelay(1000);
    oled_clear(1);
}

void oled_tar_display_page(u8 x,u8 y,u8 width,u8 *pic,u16 wordlen)
{
    u8 i;
    for(i=0;i < wordlen;i++) {
        if(i%width == 0) {
            i2c_send(0xB0+(y++),SSD1306_TRANSFER_CMD);
            i2c_send(0x0F&x,SSD1306_TRANSFER_CMD);
            i2c_send(0x10|((x&0xF0)>>4),SSD1306_TRANSFER_CMD);
        }
        if(pic == NULL) {
            i2c_send(0x00,SSD1306_TRANSFER_DATA);
            continue;
        }
        i2c_send(pic[i],SSD1306_TRANSFER_DATA);
    }
}

void oled_tar_display(u8 x,u8 y,u8 width,u8 high,u8 *pic,u16 piclen)
{
    if(pic == NULL) {
        u8 buffer[(high+8)/8*width];
        memset(buffer,0,sizeof(buffer));
        oled_tar_display(x,y,width,high,buffer,sizeof(buffer));
        // oled_tar_display_page(x,y/8,width,NULL,piclen);
        return;
    }
    static u8 offset,pagestart_y,pageend_y,ii,i;
    offset = y % 8;
    pagestart_y = y / 8;     //2
    pageend_y = (y + high) / 8;      //4
    ii = 0;
    while(pagestart_y <= pageend_y) {
        i2c_send(0xB0+(pagestart_y++),SSD1306_TRANSFER_CMD);
        i2c_send(0x0F&x,SSD1306_TRANSFER_CMD);
        i2c_send(0x10|((x&0xF0)>>4),SSD1306_TRANSFER_CMD);
        for(i = 0;i < width;i++,ii++) {
            if(ii < width) {
                i2c_send(pic[ii] << offset,SSD1306_TRANSFER_DATA);
            }else if(ii < piclen){
                i2c_send((pic[ii-width] >> (8 - offset)) | (pic[ii] << offset),SSD1306_TRANSFER_DATA);
            }else {
                i2c_send(pic[ii-width] >> (8 - offset),SSD1306_TRANSFER_DATA);
            }
        }
    }
}

void display_num(u32 num,u8 x,u8 y)
{
    if(num==0) {
        oled_tar_display(x,y,8,16,number_word[num],sizeof(number_word[num])/sizeof(number_word[0][0]));
        return;
    }

    static u8 dig = 0;
    static u16 pow_ = 0;
    dig = log10(abs(num)) + 1;
    pow_ = pow(10,dig-1);
    for( ;dig>0;dig--) {
        oled_tar_display(x,y,8,16,number_word[num/pow_],sizeof(number_word[num/pow_])/sizeof(number_word[0][0]));

        num %= pow_;
        pow_ /= 10;
        x += 8;
    }

}

void display_reverse(u8 flag) 
{
    if (flag) {
        i2c_send(0xA7,SSD1306_TRANSFER_CMD);;  // 反色显示
    } else {
        i2c_send(0xA6,SSD1306_TRANSFER_CMD);;  // 正常显示
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

void display_circle(float pitch,float roll)
{
    static u8 last_x = 0,last_y = 0;
    static u8 cur_x = 0,cur_y = 0;

    //计算偏移
    cur_x = CIRCLE_CENTER_X - roll  / 180.0f * 56.0f;
    cur_y = CIRCLE_CENTER_Y + pitch / 180.0f * 24.0f;

    //避免重复更新导致闪屏问题
    if((cur_x == last_x) && (cur_y == last_y)) {
        return;
    }
    printf(">>%.1f>>>%d====%.1f>>>%d\n",roll,cur_x,pitch,cur_y);
    // printf(">>%d====%d\n",cur_x,cur_y);

    //球和中心“十”碰撞
    if(ball_is_center(cur_x,cur_y)) {
        // printf("is crash\n");
        static u8 buffer[CIRCLE_HIGH_PAGE*CIRCLE_WIDTH] = {0};
        memcpy(buffer,circle_pic,32);
        static s8 buffer_x = 0,buffer_y = 0,center_x = 0,center_y = 0;
        // printf("=====%d=%d=====\n",CIRCLE_CENTER_X - cur_x,(CIRCLE_CENTER_Y - cur_y)/8);
        buffer_y = (CIRCLE_CENTER_Y - cur_y)/8;
        if(buffer_y < 0) {
            center_y = -buffer_y;
            buffer_y = 0;
        }else {
            center_y = 0;
        }
        for( ;MAX(buffer_y,center_y) < CIRCLE_HIGH_PAGE;buffer_y++,center_y++) {
            buffer_x = (CIRCLE_CENTER_X - cur_x);
            if(buffer_x < 0) {
                center_x = -buffer_x;
                buffer_x = 0;
            }else {
                center_x = 0;
            }
            printf("===%d %d==%d %d\n",buffer_x,buffer_y,center_x,center_y);
            for( ;MAX(buffer_x,center_x) < CIRCLE_WIDTH;buffer_x++,center_x++) {
                // printf("%d %d=%d %d\n",buffer_x,buffer_y,center_x,center_y);
                buffer[buffer_y*CIRCLE_WIDTH+buffer_x] ^= center_point[center_y*CIRCLE_WIDTH + center_x];
            }
            // printf("---------\n");
        }
        // printf("=======\n");
        
        // for(buffer_y = (CIRCLE_CENTER_Y - cur_y)/8,center_y = 0;buffer_y < CIRCLE_HIGH_PAGE;buffer_y++,center_y++) {
        //     for(buffer_x = (CIRCLE_CENTER_X - cur_x),center_x = 0;buffer_x < CIRCLE_WIDTH;buffer_x++,center_x++) {
        //         buffer[buffer_y*CIRCLE_WIDTH+buffer_x] ^= center_point[center_y*CIRCLE_WIDTH + center_x];
        //     }
        // }
        // oled_clear(0);
        //擦除旧位置，拓展2像素宽度，高度拓展至3页
        oled_tar_display_page((last_x-1 >= 0 ? last_x-1 : last_x),last_y/8,CIRCLE_WIDTH+2,NULL,54);     //54=(CIRCLE_WIDTH+2)*(CIRCLE_WIDTH/8+1)

        // oled_tar_display(56,24,16,16,center_point,sizeof(center_point));
        oled_tar_display_page(56,3,16,center_point,sizeof(center_point));
        //显示新位置
        // printf("sizeof%d %d\n",sizeof(buffer),sizeof(circle_pic));
        oled_tar_display(cur_x,cur_y,CIRCLE_WIDTH,CIRCLE_WIDTH,buffer,sizeof(buffer));
    }else {
        //擦除旧位置，拓展2像素宽度，高度拓展至3页
        oled_tar_display_page((last_x-1 >= 0 ? last_x-1 : last_x),last_y/8,CIRCLE_WIDTH+2,NULL,54);     //54=(CIRCLE_WIDTH+2)*(CIRCLE_WIDTH/8+1)
        
        oled_tar_display(56,24,16,16,center_point,sizeof(center_point));
        //显示新位置
        oled_tar_display(cur_x,cur_y,CIRCLE_WIDTH,CIRCLE_WIDTH,circle_pic,sizeof(circle_pic));
    }
    //更新旧位置
    last_x = cur_x;
    last_y = cur_y;

    if(cur_x == CIRCLE_CENTER_X && cur_y == CIRCLE_CENTER_Y) {
        display_reverse(1);
    } else {
        display_reverse(0);
    }
}
