#include "dht11.h"
#include "delay.h"
#include "lcd.h"
#include "usart.h"
//DHT11数字湿温度传感器采用单总线数据格式。单个数据引脚端口完成输入输出双向传输。其数据包由5Byte（40Bit）组成。数据分小数部分和整数部分，
//一次完整的数据传输为40bit，高位先出。

//DHT11的数据格式为：8bit湿度整数数据+8bit湿度小数数据+8bit温度整数数据+8bit温度小数数据+8bit校验和。

u8 temp1,temp2,humi1,humi2;
//复位
void DHT11_Rst1(void)
{
		DHT11_IO_OUT1();//SET OUTPUT
		DHT11_DQ_OUT1=0;//拉低数据线
		delay_ms(20);//保持至少18ms
		DHT11_DQ_OUT1=1;//拉高数据线
		delay_us(30);//保持20-40us
}

void DHT11_Rst2(void)
{
		DHT11_IO_OUT2();
		DHT11_DQ_OUT2=0;
		delay_ms(20);
		DHT11_DQ_OUT2=1;
		delay_us(30);
}
//等待回应
//等待DHT11的回应
//返回1:未检测到DHT11的存在
//返回0:存在
u8 DHT11_Check1(void)
{
		u8 retry=0;
		DHT11_IO_IN1();//设置数据口为输入
		while(DHT11_DQ_IN1&&retry<100)//等待DHT11响应	低电平为有应答  会拉低40~80us
		{
			retry++;
			delay_us(1);
		}
		if(retry>=100) return 1;//没检测到DHT11
		else retry=0;
		while(!DHT11_DQ_IN1&&retry<100)//再次等待DHT11响应  高电平为有应答  拉低后会再次拉高40~80us
		{
			retry++;
			delay_us(1);
		}
		if(retry>=100) return 1;
		else return 0;
}

//u8 DHT11_Check2(void)
//{
//		u8 retry=0;
//		DHT11_IO_IN2();
//		while(DHT11_DQ_IN2&&retry<100)
//		{
//			retry++;
//			delay_us(1);
//		}
//		if(retry>=100) return 1;
//		else retry=0;
//		while(!DHT11_DQ_IN2&&retry<100)
//		{
//			retry++;
//			delay_us(1);
//		}
//		if(retry>=100) return 1;
//		else return 0;
//}
//从DHT11读取一个位   返回值：1/0
u8 DHT11_Read_Bit1(void)
{
		u8 retry=0;
		while(DHT11_DQ_IN1&&retry<100)//等待变为低电平
		{
			retry++;
			delay_us(1);
		}
		retry=0;
		while(!DHT11_DQ_IN1&&retry<100)//等待变高电平
		{
			retry++;
			delay_us(1);
		}
		delay_us(40);//等待40us
		if(DHT11_DQ_IN1) return 1;
		else return 0;
}

u8 DHT11_Read_Bit2(void)
{
		u8 retry=0;
		while(DHT11_DQ_IN2&&retry<100)
		{
			retry++;
			delay_us(1);
		}
		retry=0;
		while(!DHT11_DQ_IN2&&retry<100)
		{
			retry++;
			delay_us(1);
		}
		delay_us(40);
		if(DHT11_DQ_IN2) return 1;
		else return 0;
}
//从DHT11读取一个字节
//返回值：读到的数据
u8 DHT11_Read_Byte1()
{
		u8 i,dat;
		dat=0;
		for(i=0;i<8;i++)
		{
				dat<<=1;
				dat|=DHT11_Read_Bit1();
		}
		return dat;
}

u8 DHT11_Read_Byte2()
{
		u8 i,dat;
		dat=0;
		for(i=0;i<8;i++)
		{
				dat<<=1;
				dat|=DHT11_Read_Bit2();
		}
		return dat;
}
//从DHT11读取一次数据
//temp:温度值(范围:0~50°)     humi:湿度值(范围:20%~90%)
//返回值：0,正常;1,读取失败
u8 DHT11_Read_Data1(u8 *temp1,u8 *humi1)//温度、湿度
{
		u8 buf1[5];
		u8 i;
		DHT11_Rst1();
		if(DHT11_Check1()==0)
		{
			for(i=0;i<5;i++)//读取40位数据  5个字节
			{
					buf1[i]=DHT11_Read_Byte1();
			}
			if(buf1[0]+buf1[1]+buf1[2]+buf1[3]==buf1[4])
			{
					*humi1=buf1[0];
					*temp1=buf1[2];
			}
		}
			else return 1;
			return 0;
}



//u8 DHT11_Read_Data2(u8 *temp2,u8 *humi2)
//{
//		u8 buf1[5];
//		u8 i;
//		DHT11_Rst2();
//		if(DHT11_Check2()==0)
//		{
//			for(i=0;i<5;i++)
//			{
//					buf1[i]=DHT11_Read_Byte2();
//			}
//			if(buf1[0]+buf1[1]+buf1[2]+buf1[3]==buf1[4])
//			{
//					*humi2=buf1[0];
//					*temp2=buf1[2];
//			}
//		}
//			else return 1;
//			return 0;
//}

u8 DHT11_Init1(void)
{
		GPIO_InitTypeDef GPIO_InitStructure1;
		
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
		
		GPIO_InitStructure1.GPIO_Pin = GPIO_Pin_0;
		GPIO_InitStructure1.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure1.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOC,&GPIO_InitStructure1);
		GPIO_SetBits(GPIOC,GPIO_Pin_0);
		
		
		DHT11_Rst1();
		return DHT11_Check1();
}

//u8 DHT11_Init2(void)
//{
//		GPIO_InitTypeDef GPIO_InitStructure;
//		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
//		
//		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
//		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//		GPIO_Init(GPIOC,&GPIO_InitStructure);
//		GPIO_SetBits(GPIOC,GPIO_Pin_13);
//		
//		
//		DHT11_Rst2();
//		return DHT11_Check2();
//}

void DHT11_test(void)
{
		u8 t=0;
		while(DHT11_Init1()) //DHT11 1初始化
		{ 
				LCD_ShowString(30,120,200,16,16,"DHT11111 Error");
				delay_ms(200);
		}
//		while(DHT11_Init2()) //DHT11 2初始化
//		{
//				LCD_ShowString(30,120,200,16,16,"DHT11222 Error");
//				delay_ms(200);
//		}
		LCD_ShowString(30,200,200,16,16,"DHT11 OK");
		LCD_ShowString(30,220,200,16,16,"TEMP1 :  C");
		LCD_ShowString(30,240,200,16,16,"HUMI1 :  %");
//		LCD_ShowString(30,260,200,16,16,"TEMP2 :  C");
//		LCD_ShowString(30,280,200,16,16,"HUMI2 :  %");
		
		while(t<100)//测试温度传感器
		{
				if(t%10==0)
				{
						DHT11_Read_Data1(&temp1,&humi1);
						//DHT11_Read_Data2(&temp2,&humi2);
						printf("temp1 %d C",temp1);
						printf("HUMI  %d %\r\n",humi1);
						//LCD_ShowNum(30+50,240,humi1,2,16);
//						LCD_ShowNum(30+50,260,temp2,2,16);
//						LCD_ShowNum(30+50,280,humi2,2,16);
						
				}
				delay_ms(10);
				t++;
		}
						LCD_ShowNum(30+50,240,humi1,2,16);
//						LCD_ShowNum(30+50,260,temp2,2,16);
//						LCD_ShowNum(30+50,280,humi2,2,16);
						
}
