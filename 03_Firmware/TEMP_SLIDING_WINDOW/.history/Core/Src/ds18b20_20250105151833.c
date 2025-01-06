
#ifndef _DS18B20_H_
#define _DS18B20_H_
#include "DS18B20.h"
temp_t temps;

#define CPU_FREQUENCY_MHZ    72		// STM32时钟主频
void delay_us(__IO uint32_t delay)
{
    int last, curr, val;
    int temp;

    while (delay != 0)
    {
        temp = delay > 900 ? 900 : delay;
        last = SysTick->VAL;
        curr = last - CPU_FREQUENCY_MHZ * temp;
        if (curr >= 0)
        {
            do
            {
                val = SysTick->VAL;
            }
            while ((val < last) && (val >= curr));
        }
        else
        {
            curr += CPU_FREQUENCY_MHZ * 1000;
            do
            {
                val = SysTick->VAL;
            }
            while ((val <= last) || (val > curr));
        }
        delay -= temp;
    }
}


void delay_ms(__IO uint32_t Delay)
{
	HAL_Delay(Delay);
}

//复位DS18B20
void DS18B20_Rst(void)
{
CLR_DS18B20();    //拉低DQ
delay_us(750);    //拉低750us
SET_DS18B20();;   //DQ=1
delay_us(15);     //15US
}


//等待DS18B20的回应
//返回1:未检测到ds18b20的存在
//返回0:存在
uint8_t DS18B20_Check(void)
{
uint8_t retry=0;

while (DS18B20_DQ_IN&&retry<200)
{
    retry++;
    delay_us(1);
};
if(retry>=200)return 1;
else retry=0;
while (!DS18B20_DQ_IN&&retry<240)
{
    retry++;
    delay_us(1);
};
if(retry>=240)return 1;
return 0;
}


//从DS18B20读取一个位
//返回值:1/0
uint8_t DS18B20_Read_Bit(void)           // read one bit
{
uint8_t data;

CLR_DS18B20();   //拉低DQ
delay_us(2);
SET_DS18B20();;  //DQ=1
delay_us(12);
if(DS18B20_DQ_IN)data=1;
else data=0;
delay_us(50);

return data;
}


//从ds18b20读取一个字节
//返回值:读取到的数据
uint8_t DS18B20_Read_Byte(void)    // read one byte
{
uint8_t i,j,dat;
dat=0;

for (i=1;i<=8;i++)
{
    j=DS18B20_Read_Bit();
    dat=(j<<7)|(dat>>1);
}
return dat;
}

//*本程序由 www.stm32cube.com 提供
//写一个字节到DS18B20
//dat:要写入的字节
void DS18B20_Write_Byte(uint8_t dat)
{
uint8_t j;
uint8_t testb;

for (j=1;j<=8;j++)
{
    testb=dat&0x01;
    dat=dat>>1;
    if (testb)
    {
        CLR_DS18B20(); //DS18B20_DQ_OUT=0;// Write 1
        delay_us(2);
        SET_DS18B20(); //DS18B20_DQ_OUT=1;
        delay_us(60);
    }
    else
    {
        CLR_DS18B20(); //DS18B20_DQ_OUT=0;// Write 0
        delay_us(60);
        SET_DS18B20(); //DS18B20_DQ_OUT=1;
        delay_us(2);
    }
}
}


//开始温度转换
void DS18B20_Start(void)// ds1820 start convert
{
DS18B20_Rst();
  DS18B20_Check();//1ms
DS18B20_Write_Byte(0xcc);// skip rom //496us
DS18B20_Write_Byte(0x44);// convert//496us
}


//初始化DS18B20的IO口DQ同时检查DS的存在
//返回1:不存在
//返回0:存在
uint8_t DS18B20_Init(void)
{
//配置PA2口
GPIO_InitTypeDef GPIO_InitStruct;

/* GPIO Ports Clock Enable */
__GPIOA_CLK_ENABLE();

/*Configure GPIO pin : PA2 */
GPIO_InitStruct.Pin = DS18B20_BIT;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
HAL_GPIO_Init(DS18B20_PORT, &GPIO_InitStruct);

SET_DS18B20();         //使PA2输出高电平

DS18B20_Rst();
return DS18B20_Check();
}


//从ds18b20得到温度值
//精度：0.1C
//返回值：温度值 （-550~1250）
float DS18B20_Get_Temp(void)
{
    uint8_t temp;
    uint8_t TL,TH;
	int16_t tem;
    DS18B20_Start ();   // ds1820 start convert  2ms
    DS18B20_Rst();
    DS18B20_Check();  //1ms
    DS18B20_Write_Byte(0xcc);// skip rom
    DS18B20_Write_Byte(0xbe);// convert   //1ms
    TL=DS18B20_Read_Byte(); // LSB
    TH=DS18B20_Read_Byte(); // MSB  1.6ms

    if(TH>7)
    {
        TH=~TH;
        TL=~TL;
        temp=0;//温度为负
    }else temp=1;//温度为正
    tem=TH; //获得高八位
    tem<<=8;
    tem+=TL;//获得低八位
    tem=(float)tem*0.625;//转换
	if(temp)return tem/10.00; //返回温度值
	else return -tem/10.00;
}

#endif // !_DS18B20_H_