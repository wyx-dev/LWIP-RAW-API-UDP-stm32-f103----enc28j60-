#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"	 
#include "usmart.h"
#include "sram.h"
#include "malloc.h"
#include "enc28j60.h" 	 
#include "lwip/netif.h"
#include "lwip_comm.h"
#include "lwipopts.h"
#include "timer.h"
#include "udp_demo.h"
#include "dht11.h"
#include "exti.h"
#include "elengine.h"
#include "motor.h"
/************************************************
 ALIENTEK ENC28J60 网络实验
 基于RAW API的UDP实验
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/

extern u8 udp_demo_flag;  //UDP 测试全局状态标记变量

int main(void)
{	 
	u8 key;
	delay_init();	    		//延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 	//设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(115200);	 		//串口初始化为115200
	KEY_Init();	 				//初始化按键
	LED_Init();			   	 	//LED端口初始化
	TIM3_Int_Init(1000,719);	//定时器3频率为100hz  //系统时钟  用来保持LWIP精准计时
 	MOTOR_PWM_Init(7199,0);   //=====初始化PWM 10KHZ，用于驱动电机 如需初始化电调接口 改为MiniBalance_PWM_Init(9999,35) 200HZ	
 	FSMC_SRAM_Init();			//初始化外部SRAM
	my_mem_init(SRAMIN);		//初始化内部内存池
	my_mem_init(SRAMEX);		//初始化外部内存池
	
	while(lwip_comm_init()) 	//lwip初始化  TCP/IP协议栈
	{
		delay_ms(1200);
	}
	printf("LWIP Init Success!\r\n");
#if LWIP_DHCP   //使用DHCP
	while((lwipdev.dhcpstatus!=2)&&(lwipdev.dhcpstatus!=0XFF))//等待DHCP获取成功/超时溢出
	{
		lwip_periodic_handle();	//LWIP内核需要定时处理的函数
	}
#endif
	delay_ms(500);			//延时1s
	delay_ms(500);
	udp_demo_test();  		//UDP 模式  连接  连接成功就在此函数
	//连接失败才会执行 下面while(1)用来再次连接上位机
  	while(1)
	{	
		key = KEY_Scan(0);
		if(key == KEY1_PRES)		//按KEY1键建立连接
		{
			if((udp_demo_flag & 1<<5)) printf("UDP连接已经建立,不能重复连接\r\n");	//如果连接成功,不做任何处理
			else udp_demo_test();		//当断开连接后,调用udp_demo_test()函数
		}
		delay_ms(10);
	}
}


