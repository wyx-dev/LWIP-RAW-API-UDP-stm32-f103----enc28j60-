#include "udp_demo.h" 
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "malloc.h"
#include "stdio.h"
#include "string.h" 
#include "dht11.h"
#include "led.h"
#include "elengine.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK ENC28J60模块
//UDP 测试代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/4/30
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//*******************************************************************************
//修改信息
//无
////////////////////////////////////////////////////////////////////////////////// 	   
 
//UDP接收数据缓冲区
u8 udp_demo_recvbuf[UDP_DEMO_RX_BUFSIZE];	//UDP接收数据缓冲区 
//UDP发送数据内容
u8 *udp_demo_sendbuf=NULL;
u8 elengine_wyx = 1;

extern u8 temp1,temp2,humi1,humi2;
//UDP 测试全局状态标记变量
//bit7:没有用到
//bit6:0,没有收到数据;1,收到数据了.
//bit5:0,没有连接上;1,连接上了.
//bit4~0:保留
u8 udp_demo_flag;
struct sardata sendrecv;
//初始化一个sendrecv
void initsardata(void)
{
	u8 i;
	sendrecv.head[0]=0x5A;//Z
	sendrecv.head[1]=0x4E;//N
	sendrecv.head[2]=0x4A;//J
	sendrecv.head[3]=0x4A;//J  相当于前缀
	sendrecv.length=15;
	
	for(i=0;i<8;i++)
	{
		sendrecv.data[i]=0x00;
	}
	sendrecv.humi[0]=0x00;//传感器0状态
	sendrecv.humi[1]=0x00;//传感器1状态
	sendrecv.elengine[0]=0x00;//电机0状态
	sendrecv.elengine[1]=0x00;//电机1状态
	sendrecv.led[0]=0x00;//led0状态
	sendrecv.led[1]=0x00;//led1状态
	sendrecv.led[2]=0x00;//led2状态
}

//设置远端IP地址
void udp_demo_set_remoteip(void)
{
	u8 *tbuf;
	u16 xoff;
	u8 key;
	POINT_COLOR=RED;
	  
	tbuf=mymalloc(SRAMIN,100);	//申请内存
	if(tbuf==NULL)return;
	//前三个IP保持和DHCP得到的IP一致  同一网段
	lwipdev.remoteip[0]=lwipdev.ip[0];//192
	lwipdev.remoteip[1]=lwipdev.ip[1];//168
	lwipdev.remoteip[2]=lwipdev.ip[2];//1
	lwipdev.remoteip[3]=(uint8_t)(0x01);//自己定义服务器端IP
	sprintf((char*)tbuf,"Remote IP:%d.%d.%d.",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2]);//远端IP
	POINT_COLOR=BLUE;
	xoff=strlen((char*)tbuf)*8+30;
	printf("KEY1:+  KEY0:-\r\n");  
	printf("KEY_UP:OK\r\n");
	while(1)
	{
		
		key=KEY_Scan(0);
		if(key==WKUP_PRES)break;
		else if(key)
		{
			if(key==KEY1_PRES)lwipdev.remoteip[3]++;//IP增加
			if(key==KEY0_PRES)lwipdev.remoteip[3]--;//IP减少
			printf("%d.%d.%d.%d\r\n",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);//显示新IP
		}
	} 
	myfree(SRAMIN,tbuf);
} 

//UDP测试
void udp_demo_test(void)
{
 	err_t err;
	struct udp_pcb *udppcb;  	//定义一个UDP服务器控制块
	struct ip_addr rmtipaddr;  	//用来保存远端ip地址
 	
	u8 *tbuf;
 	u8 key;
	u8 res=0;		
	udp_demo_set_remoteip();//设置远端IP
	tbuf=mymalloc(SRAMIN,200);	//申请内存
	if(tbuf==NULL)
		return ;		//内存申请失败了,直接退出
	sprintf((char*)tbuf,"Local IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);//服务器IP
	sprintf((char*)tbuf,"Remote IP:%d.%d.%d.%d",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);//远端IP
	sprintf((char*)tbuf,"Remote Port:%d",UDP_DEMO_PORT);//客户端端口号
	POINT_COLOR=BLUE;//字体为蓝色
	//创建一个UDP控制块
	udppcb=udp_new();//他自己的内存池创建
	initsardata();//初始化发送包
	if(udppcb)//创建成功
	{ 
		IP4_ADDR(&rmtipaddr,lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);//打包成u32 位
		err=udp_connect(udppcb,&rmtipaddr,UDP_DEMO_PORT);//UDP客户端连接到指定IP地址和端口号的服务器
		if(err==ERR_OK)
		{
			err=udp_bind(udppcb,IP_ADDR_ANY,UDP_DEMO_PORT);//绑定本地IP地址与端口号
			if(err==ERR_OK)	//绑定完成
			{
				udp_recv(udppcb,udp_demo_recv,NULL);//注册接收回调函数  给相关的udppcb结构体元素赋值
				printf("STATUS:Connected   \r\n");//标记连接上了(UDP是非可靠连接,这里仅仅表示本地UDP已经准备好)
				udp_demo_flag |= 1<<5;			//标记已经连接上
				POINT_COLOR=RED;
				POINT_COLOR=BLUE;//蓝色字体
			}else res=1;
		}else res=1;		
	}else res=1;
	while(res==0)//连接成功
	{
		key=KEY_Scan(0);
		if(key==WKUP_PRES)break;
		if(key==KEY0_PRES)//KEY0按下了,发送数据  用来测试是否连通
		{
			udp_demo_senddata(udppcb);
		}	
		if(udp_demo_flag&1<<6)//是否收到数据?
		{
			udp_demo_senddata(udppcb);
			elengine_wyx = 1;
			udp_demo_flag&=~(1<<6);//标记数据已经被处理了.
		} 
		lwip_periodic_handle();//轮询任务 维持LWIP内核
		delay_ms(2);
	}
	udp_demo_connection_close(udppcb); 
	myfree(SRAMIN,tbuf);
} 

//UDP回调函数
//p为原始数据
//
void udp_demo_recv(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port)
{
	u32 data_len = 0;
	struct pbuf *q;
	if(p!=NULL)	//接收到不为空的数据时
	{
		memset(udp_demo_recvbuf,0,UDP_DEMO_RX_BUFSIZE);  //数据接收缓冲区清零 
		for(q=p;q!=NULL;q=q->next)  //遍历完整个pbuf链表  将p里面的数据拷贝到udp_demo_recvbuf缓冲区
		{
			//判断要拷贝到UDP_DEMO_RX_BUFSIZE中的数据是否大于UDP_DEMO_RX_BUFSIZE的剩余空间，如果大于
			//的话就只拷贝UDP_DEMO_RX_BUFSIZE中剩余长度的数据，否则的话就拷贝所有的数据
			if(q->len > (UDP_DEMO_RX_BUFSIZE-data_len)) memcpy(udp_demo_recvbuf+data_len,q->payload,(UDP_DEMO_RX_BUFSIZE-data_len));//拷贝数据
			else memcpy(udp_demo_recvbuf+data_len,q->payload,q->len);
			data_len += q->len;  	
			if(data_len > UDP_DEMO_RX_BUFSIZE) break; //超出TCP客户端接收数组,跳出	
		}
		readrecv();//读取接受缓冲区的东西，以便处理
		//需修改的话，修改此处
		Message_processing();
		/////////
		upcb->remote_ip=*addr; 				//记录远程主机的IP地址
		upcb->remote_port=port;  			//记录远程主机的端口号
		lwipdev.remoteip[0]=upcb->remote_ip.addr&0xff; 		//IADDR4
		lwipdev.remoteip[1]=(upcb->remote_ip.addr>>8)&0xff; //IADDR3
		lwipdev.remoteip[2]=(upcb->remote_ip.addr>>16)&0xff;//IADDR2
		lwipdev.remoteip[3]=(upcb->remote_ip.addr>>24)&0xff;//IADDR1 
		udp_demo_flag|=1<<6;	//标记接收到数据了
		pbuf_free(p);//释放内存
	}else
	{
		udp_disconnect(upcb);
		udp_demo_flag &= ~(1<<5);	//标记连接断开
	} 
} 
//UDP服务器发送数据
void udp_demo_senddata(struct udp_pcb *upcb)
{
	struct pbuf *ptr;
	ptr=pbuf_alloc(PBUF_TRANSPORT,strlen((char*)udp_demo_sendbuf),PBUF_POOL); //申请内存
	if(ptr)//申请成功
	{
		DHT11_Read_Data1(&temp1,&humi1);
		udp_demo_sendbuf=mymalloc(SRAMIN,sendrecv.length+1);
		setdata();
		setsendbuf();	//将被设置好的 sendrecv赋给udp_demo_sendbuf
		printf(":%s  %d\r\n",udp_demo_sendbuf,strlen((char*)udp_demo_sendbuf));
		pbuf_take(ptr,(char*)udp_demo_sendbuf,strlen((char*)udp_demo_sendbuf));//构造ptr的数据部分
		udp_send(upcb,ptr);	//udp发送数据 
		pbuf_free(ptr);//释放内存
		myfree(SRAMIN,udp_demo_sendbuf);
	} 
} 
//关闭UDP连接
void udp_demo_connection_close(struct udp_pcb *upcb)
{
	udp_disconnect(upcb); 
	udp_remove(upcb);			//断开UDP连接 
	udp_demo_flag &= ~(1<<5);	//标记连接断开
	POINT_COLOR = RED;
	
	POINT_COLOR=BLUE;
}

void setdata(void)
{
	//temp整数部分
	if(((temp1&0xf0)>>4)>=0x00&&((temp1&0xf0)>>4)<0x0A)
		sendrecv.data[0]=((temp1&0xf0)>>4)+0x30;
	else
		sendrecv.data[0]=(temp1&0xf0)+0x37;
	//temp小数部分
	if((temp1&0x0f)>=0x00&&(temp1&0x0f)<0x0A)	
		sendrecv.data[1]=(temp1&0x0f)+0x30;
	else
		sendrecv.data[1]=(temp1&0x0f)+0x37;
	//humi整数
	if(((humi1&0xf0)>>4)>=0x00&&((humi1&0xf0)>>4)<0x0A) 
		sendrecv.data[2]=((humi1&0xf0)>>4)+0x30;
	else
		sendrecv.data[2]=(humi1&0xf0)+0x37;
	//humi小数
	if((humi1&0x0f)>=0x00&&(humi1&0x0f)<0x0A)
		sendrecv.data[3]=(humi1&0x0f)+0x30;
	else
		sendrecv.data[3]=(humi1&0x0f)+0x37;
	printf("humi:%d temp: %d sendrecv.data:%s\r\n",humi1,temp1,sendrecv.data);
}

u8 judge(void)
{
		u8 position;
		for(position=0;position<sendrecv.length;position++)
		{
				if(udp_demo_recvbuf[position]==0x5A)//Z
				if(udp_demo_recvbuf[position+1]==0x4E)//N
				if(udp_demo_recvbuf[position+2]==0x4A)//J
				if(udp_demo_recvbuf[position+3]==0x4A)//J
					break;
		}
		if(position>=sendrecv.length) return 0;
		else return position+4;//让position指向第一个有效数据位
}

void readrecv(void)
{
	u8 position;
	if(judge()==0);//判断数据是否符合ZNJJ 0 不符合  1  符合
	else//符合做相关处理
	{
		position=judge();//从该位提取控制数据
		sendrecv.led[0]=udp_demo_recvbuf[position];
		position++;
		sendrecv.led[1]=udp_demo_recvbuf[position];
		position++;
		sendrecv.led[2]=udp_demo_recvbuf[position];
		position++;
		sendrecv.elengine[0]=udp_demo_recvbuf[position];
		position++;
		sendrecv.elengine[1]=udp_demo_recvbuf[position];
		position++;
		sendrecv.humi[0]=((udp_demo_recvbuf[position]-0x30));
		position++;
		sendrecv.humi[1]=((udp_demo_recvbuf[position]-0x30));
	}
}

void setsendbuf(void)
{
	u8 i;
	udp_demo_sendbuf[0]=sendrecv.head[0];
	udp_demo_sendbuf[1]=sendrecv.head[1];
	udp_demo_sendbuf[2]=sendrecv.head[2];
	udp_demo_sendbuf[3]=sendrecv.head[3];
	udp_demo_sendbuf[9]=sendrecv.humi[0]+0x30;
	udp_demo_sendbuf[10]=sendrecv.humi[1]+0x30;
	for(i=11;i<15;i++)
	{
		udp_demo_sendbuf[i]=sendrecv.data[i-11];
	}
	
	udp_demo_sendbuf[7]=sendrecv.elengine[0];
	udp_demo_sendbuf[8]=sendrecv.elengine[1];
	udp_demo_sendbuf[4]=sendrecv.led[0];
	udp_demo_sendbuf[5]=sendrecv.led[1];
	udp_demo_sendbuf[6]=sendrecv.led[2];
}
//相关处理函数自己写的  对灯电机的操作。修改此处
void Message_processing()
{		
	printf("l:%s e:%s %s  \r\n",sendrecv.led,sendrecv.elengine,udp_demo_recvbuf);
	if(sendrecv.led[0]=='1')           	LED0=1;
	else 							 	LED0=0;
	if(sendrecv.led[1]=='1')          	LED1=1;
	else 							 	LED1=0;
	if(sendrecv.led[2]=='1')           	LED2=1;
	else 							 	LED2=0;
	//printf("wwwwwwwww%d  \r\n",sendrecv.elengine[0]);
	if(sendrecv.humi[0]==1){
		//printf("wwwwwwwww%d  \r\n",sendrecv.humi[0]);
		if(sendrecv.elengine[0]=='1'&&sendrecv.elengine[1]=='1'&&elengine_wyx)//控制发动机
		{
			Open_Elengine_1(1);
			GPIO_ResetBits(GPIOB,GPIO_Pin_12);
			GPIO_ResetBits(GPIOB,GPIO_Pin_13);
			//printf("qqqqqqqqq%d  %d\r\n",sendrecv.elengine[0],sendrecv.elengine[1]);
			elengine_wyx = 0;
		}
		if(sendrecv.elengine[0]=='1'&&sendrecv.elengine[1]=='0'&&elengine_wyx)//控制发动机 
		{
			Open_Elengine_1(0);
			//printf("wwwwwwwww%d  \r\n",sendrecv.elengine[0]);
			elengine_wyx = 0;
		}
	}
	else
	{
		//printf("22222222%d \r\n",sendrecv.humi[0]);
		Close_Elengine_1();
	}
	if(sendrecv.elengine[0]=='0'&&sendrecv.elengine[1]=='1')//控制发动机   
	{
		//printf("eeeeeeeee%d  %d\r\n",sendrecv.elengine[0],sendrecv.elengine[1]);
		Open_Elengine_2();			
	}
	if(sendrecv.elengine[0]=='0'&&sendrecv.elengine[1]=='0')//控制发动机  
	{
		Close_Elengine_2();
		//printf("rrrrrrrrr%d  %d\r\n",sendrecv.elengine[0],sendrecv.elengine[1]);
	}
}
