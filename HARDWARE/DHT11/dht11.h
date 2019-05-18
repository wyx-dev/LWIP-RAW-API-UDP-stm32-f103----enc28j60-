#ifndef _DHT11_H
#define _DHT11_H
#include "sys.h"

//IO方向设置
#define DHT11_IO_IN1()  {GPIOC->CRL&=0XFFFFFFF0;GPIOC->CRL|=8<<0;}
#define DHT11_IO_OUT1() {GPIOC->CRL&=0XFFFFFFF0;GPIOC->CRL|=3<<0;}
#define DHT11_IO_IN2()  {GPIOC->CRH&=0XFF0FFFFF;GPIOC->CRH|=8<<20;}
#define DHT11_IO_OUT2() {GPIOC->CRH&=0XFF0FFFFF;GPIOC->CRH|=3<<20;}
//IO操作函数
#define DHT11_DQ_OUT1 PCout(0)
#define DHT11_DQ_IN1  PCin(0)
#define DHT11_DQ_OUT2 PCout(13)
#define DHT11_DQ_IN2  PCin(13)

extern u8 temp1,temp2,humi1,humi2;

u8 DHT11_Init1(void);//初始化DHT11
u8 DHT11_Init2(void);
u8 DHT11_Read_Data1(u8 *temp1,u8 *humi1);//读取温湿度
u8 DHT11_Read_Data2(u8 *temp1,u8 *humi1);
u8 DHT11_Read_Byte1(void);//读一个字节
u8 DHT11_Read_Bit1(void);//读一个位
u8 DHT11_Read_Byte2(void);//读一个字节
u8 DHT11_Read_Bit2(void);//读一个位
u8 DHT11_Check1(void);//检测是否存在DHT11
u8 DHT11_Check2(void);
void DHT11_Rst1(void);//复位DHT11
void DHT11_Rst2(void);
void DHT11_test(void);
#endif
