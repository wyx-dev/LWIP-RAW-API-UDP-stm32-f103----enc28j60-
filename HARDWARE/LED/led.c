#include "led.h"
#include "exti.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK miniSTM32开发板
//LED驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/2
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	   

//初始化PB5和PE5为输出口.并使能这两个口的时钟		    
//LED IO初始化
void LED_Init(void)
{
	RCC->APB2ENR|=1<<4;			//使能PORTC时钟
	GPIOC->CRH&=0xFFFFF000;
	GPIOC->CRH|=0x00000333;		//PC8 9 10
	
	GPIOC->ODR&=~(7<<8);		//PA 12 
	PCout(8) = 0;				
	PCout(9) = 0;
	PCout(10) = 0;				//三个都拉低
}

