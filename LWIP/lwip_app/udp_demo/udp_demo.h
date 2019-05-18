#ifndef __UDP_DEMO_H
#define __UDP_DEMO_H
#include "sys.h"
#include "lwip_comm.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK ENC28J60模块
//UDP 测试代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/3/15
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//*******************************************************************************
//修改信息
//无
////////////////////////////////////////////////////////////////////////////////// 	   
 
#define UDP_DEMO_RX_BUFSIZE		2000	//定义udp最大接收数据长度 
#define UDP_DEMO_PORT			8099	//定义udp连接的端口 
//发送数据包
struct sardata
{
	u8  head[4]; //标志头
	u16 length;  //长度
	u8  humi[2];    //温湿度传感器状态
	u8  data[4]; //温度湿度数据
	u8  elengine[2];//电机及状态
	u8  led[3];     //led灯
};

void initsardata(void);
void setdata(void);
u8 judge(void);
void readrecv(void);
void setsendbuf(void);
void udp_demo_test(void);
void udp_demo_recv(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port);
void udp_demo_senddata(struct udp_pcb *upcb);
void udp_demo_connection_close(struct udp_pcb *upcb);
void Message_processing(void);
#endif

