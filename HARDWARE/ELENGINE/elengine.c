#include "elengine.h"
#include "sys.h"
#include "delay.h"

void dianjiqudong()
{
	 GPIO_InitTypeDef  GPIO_InitStructure;
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //使能PA,PD端口时钟
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;				 //PB12  13  14  15端口配置
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	 GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOA.8
	 GPIO_ResetBits(GPIOB, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);						 //PB 12  13  14  15 输出低
}
//打开电机1函数
void Open_Elengine_1(int dir)
{
	u16 i=0;
	if(dir)//正转
	{
			GPIO_SetBits(GPIOB,GPIO_Pin_13);
			GPIO_ResetBits(GPIOB,GPIO_Pin_12);
			TIM1->CCR4 = 3000;//电机1转动速度调节
			delay_ms(1000);
	}
	else//反转
	{
			GPIO_SetBits(GPIOB,GPIO_Pin_12);
			GPIO_ResetBits(GPIOB,GPIO_Pin_13);
			TIM1->CCR4 = 3000;//电机1转动速度调节
		delay_ms(1000);
	}
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);
	GPIO_ResetBits(GPIOB,GPIO_Pin_13);
}
//打开电机2函数
void Open_Elengine_2(void)
{
	GPIO_SetBits(GPIOB,GPIO_Pin_14);
	GPIO_ResetBits(GPIOB,GPIO_Pin_15);
	TIM1->CCR1 = 2000;//电机1转动速度调节
}
//关闭电机1函数
void Close_Elengine_1(void)
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);
	GPIO_ResetBits(GPIOB,GPIO_Pin_13);
	TIM1->CCR4 = 0;//电机1转动速度调节
}
//关闭电机2函数
void Close_Elengine_2(void)
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_14);
	GPIO_ResetBits(GPIOB,GPIO_Pin_15);
	TIM1->CCR1 = 0;//电机1转动速度调节
}
