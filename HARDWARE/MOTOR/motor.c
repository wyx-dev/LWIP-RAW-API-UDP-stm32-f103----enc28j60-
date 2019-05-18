#include "motor.h"
#include "delay.h"




//电机PWM初始化
//arr：自动重装值
//psc：时钟预分频数
//定时器1 通道1，通道4
void MOTOR_PWM_Init(u16 arr,u16 psc)
{   
	RCC->APB2ENR|=1<<11;    		//使能TIM1时钟	
	RCC->APB2ENR|=1<<2;    			//使能PORTA时钟
	RCC->APB2ENR|=1<<3;    			//使能PORTB时钟
	//电机PWM引脚使能
	GPIOA->CRH&=0XFFFF0FF0;			//PA8、11输出	
	GPIOA->CRH|=0X0000B00B;			//复用功能输出 	

	TIM1->ARR=arr;             	//设定计数器自动重装值 
	TIM1->PSC=psc;             		//预分频器不分频
	TIM1->CCMR2|=6<<12;        		//CH4 PWM1模式	
	TIM1->CCMR1|=6<<4;         		//CH1 PWM1模式	
	TIM1->CCMR2|=1<<11;        		//CH4预装载使能	 
	TIM1->CCMR1|=1<<3;         		//CH1预装载使能	  
	TIM1->CCER|=1<<12;         		//CH4输出使能	   
	TIM1->CCER|=1<<0;          		//CH1输出使能	
	TIM1->BDTR |= 1<<15;       		//TIM1必须要这句话才能输出PWM
	TIM1->CR1=0x8000;          		//ARPE使能 
	TIM1->CR1|=0x01;          		//使能定时器1 	
}


