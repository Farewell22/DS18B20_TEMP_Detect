#ifndef __DS18B20_H
#define __DS18B20_H

#include "stm32f1xx_hal.h"
#include "main.h"

#define DS18B20_BIT        	DS18_Pin

#define DS18B20_PORT         DS18_GPIO_Port

#define CLR_DS18B20()       HAL_GPIO_WritePin (DS18B20_PORT,DS18B20_BIT,GPIO_PIN_RESET)

#define SET_DS18B20()       HAL_GPIO_WritePin (DS18B20_PORT,DS18B20_BIT,GPIO_PIN_SET)

#define DS18B20_DQ_IN   HAL_GPIO_ReadPin(DS18B20_PORT, DS18B20_BIT)
void delay_us(uint32_t value);
void delay_ms(__IO uint32_t Delay);
uint8_t DS18B20_Init(void);          //初始化DS18B20
float DS18B20_Get_Temp(void);   //获取温度
void DS18B20_Start(void);       //开始温度转换
void DS18B20_Write_Byte(uint8_t dat);//写入一个字节
uint8_t DS18B20_Read_Byte(void);     //读出一个字节
uint8_t DS18B20_Read_Bit(void);      //读出一个位
uint8_t DS18B20_Check(void);         //检测是否存在DS18b20
void DS18B20_Rst(void);         //复位DS18B20
#define BUFFER_SIZE 20
#define SLIDING_WINDOW
typedef struct 
{
    uint8_t temps_cur;
    uint8_t buffer_cur;
    float warning_temp;
    float temps_buffer[WINDOW_SIZE];
    float buffer[BUFFER_SIZE];
    /* data */
}temp_t;
extern temp_t temps;

#endif
