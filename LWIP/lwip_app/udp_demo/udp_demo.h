#ifndef __UDP_DEMO_H
#define __UDP_DEMO_H
#include "sys.h"
#include "lwip_comm.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ���������ɣ��������������κ���;
//ALIENTEK ENC28J60ģ��
//UDP ���Դ���	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2015/3/15
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) �������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//*******************************************************************************
//�޸���Ϣ
//��
////////////////////////////////////////////////////////////////////////////////// 	   
 
#define UDP_DEMO_RX_BUFSIZE		2000	//����udp���������ݳ��� 
#define UDP_DEMO_PORT			8099	//����udp���ӵĶ˿� 
//�������ݰ�
struct sardata
{
	u8  head[4]; //��־ͷ
	u16 length;  //����
	u8  humi[2];    //��ʪ�ȴ�����״̬
	u8  data[4]; //�¶�ʪ������
	u8  elengine[2];//�����״̬
	u8  led[3];     //led��
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
