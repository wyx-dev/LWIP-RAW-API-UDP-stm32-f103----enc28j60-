[LWIP的RAW API UDP通信详解(stm32f103---enc28j60)](https://blog.csdn.net/weixin_42149196/article/details/90246815)
[项目整体托管到github上](https://github.com/WyxLOVES/LWIP-RAW-API-UDP-stm32-f103----enc28j60-)
#  实现任务
通过接收到上位机端发送来的数据来实现控制开发板做相应的操作。

#  代码实现
###  首先看看几个主要的结构体
//发送数据包
~~~c
struct sardata
{
	u8  head[4]; //标志头
	u16 length;  //长度
	u8  humi[2];    //温湿度传感器状态
	u8  data[4]; //温度湿度数据
	u8  elengine[2];//电机及状态
	u8  led[3];     //led灯
};
//初始化上述结构体函数，用来发送数据包到上位机
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
~~~
数据包是根据我们和服务器端所定的协议来构建的。如下图
![和上位机所定的协议](https://img-blog.csdnimg.cn/20190516183614441.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl80MjE0OTE5Ng==,size_16,color_FFFFFF,t_70)
udp_pcb udp协议控制块（用来装一些双方连接信息）
~~~ c
struct udp_pcb {
/* Common members of all PCB types */
  IP_PCB;

/* 指向下一个PCB */

  struct udp_pcb *next;

  u8_t flags;
  /** 端口 本地和远端定义的端口号*/
  u16_t local_port, remote_port;

#if LWIP_IGMP
  /** outgoing network interface for multicast packets */
  ip_addr_t multicast_ip;
#endif /* LWIP_IGMP */

#if LWIP_UDPLITE
  /** used for UDP_LITE only */
  u16_t chksum_len_rx, chksum_len_tx;
#endif /* LWIP_UDPLITE */

  /** 接受回调函数*/
  udp_recv_fn recv;
  /** user-supplied argument for the recv callback 用户提供的回调函数 */
  void *recv_arg;  
};
~~~
- ip_addr 32位的IP地址 
- 设置远端IP地址
~~~c
/* This is the aligned version of ip_addr_t,
   used as local variable, on the stack, etc. */
struct ip_addr {
  u32_t addr;
};
/*此函数用来设置远端IP地址
*/
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
	//此处使用可调节式的远端IP设置
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

~~~
 1. 本地IP可从上述代码看出，本地IP采用DHCP动态分配的IP地址，如果分配不成功则使用默认设置IP，如下代码所示，还初始化了MAC、默认远端IP、默认子网掩码、默认网关。lwip控制结构体主要对上述结构体进行封装。
```c
//lwip控制结构体
typedef struct  
{
	u8 mac[6];      //MAC地址
	u8 remoteip[4];	//远端主机IP地址 
	u8 ip[4];       //本机IP地址
	u8 netmask[4]; 	//子网掩码
	u8 gateway[4]; 	//默认网关的IP地址
	
	vu8 dhcpstatus;	//dhcp状态 
					//0,未获取DHCP地址;
					//1,进入DHCP获取状态
					//2,成功获取DHCP地址
					//0XFF,获取失败.
}__lwip_dev;
//lwip 默认IP设置
//lwipx:lwip控制结构体指针
void lwip_comm_default_ip_set(__lwip_dev *lwipx)
{
	//默认远端IP为:192.168.1.100
	lwipx->remoteip[0]=192;	
	lwipx->remoteip[1]=168;
	lwipx->remoteip[2]=1;
	lwipx->remoteip[3]=100;
	//MAC地址设置(高三字节固定为:2.0.0,低三字节用STM32唯一ID)
	lwipx->mac[0]=enc28j60_dev.macaddr[0];
	lwipx->mac[1]=enc28j60_dev.macaddr[1];
	lwipx->mac[2]=enc28j60_dev.macaddr[2];
	lwipx->mac[3]=enc28j60_dev.macaddr[3];
	lwipx->mac[4]=enc28j60_dev.macaddr[4];
	lwipx->mac[5]=enc28j60_dev.macaddr[5]; 
	//默认本地IP为:192.168.1.30
	lwipx->ip[0]=192;	
	lwipx->ip[1]=168;
	lwipx->ip[2]=1;
	lwipx->ip[3]=30;
	//默认子网掩码:255.255.255.0
	lwipx->netmask[0]=255;	
	lwipx->netmask[1]=255;
	lwipx->netmask[2]=255;
	lwipx->netmask[3]=0;
	//默认网关:192.168.1.1
	lwipx->gateway[0]=192;	
	lwipx->gateway[1]=168;
	lwipx->gateway[2]=1;
	lwipx->gateway[3]=1;	
	lwipx->dhcpstatus=0;//没有DHCP	
} 
```
##  int main(void)
主函数我们主要看下lwip_comm_init();与udp_demo_test();这两个函数。
```c
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
```
###  u8 lwip_comm_init(void)
*此函数的执行流程*
1. 初始化enc28j60模块
2. 初始化lwip内核
3. 设置默认IP等信息
4. 如果使用DHCP，则开启DHCP服务并分配IP等信息
5. 添加网口
6. 设置网口为默认网口
7. 打开网口
```c
//LWIP初始化(LWIP启动的时候使用)
//返回值:0,成功
//      1,内存错误
//      2,DM9000初始化失败
//      3,网卡添加失败.
u8 lwip_comm_init(void)
{
	struct netif *Netif_Init_Flag;		//调用netif_add()函数时的返回值,用于判断网络初始化是否成功
	struct ip_addr ipaddr;  			//ip地址
	struct ip_addr netmask; 			//子网掩码
	struct ip_addr gw;      			//默认网关 
	if(lwip_comm_mem_malloc())return 1;	//内存申请失败
	if(ENC28J60_Init())return 2;		//初始化ENC28J60
	lwip_init();						//初始化LWIP内核
	lwip_comm_default_ip_set(&lwipdev);	//设置默认IP等信息

#if LWIP_DHCP		//使用动态IP
	ipaddr.addr = 0;
	netmask.addr = 0;
	gw.addr = 0;
#else				//使用静态IP
	IP4_ADDR(&ipaddr,lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	IP4_ADDR(&netmask,lwipdev.netmask[0],lwipdev.netmask[1] ,lwipdev.netmask[2],lwipdev.netmask[3]);
	IP4_ADDR(&gw,lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
	printf("网卡en的MAC地址为:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
	printf("静态IP地址........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	printf("子网掩码..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
	printf("默认网关..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
#endif
	Netif_Init_Flag=netif_add(&lwip_netif,&ipaddr,&netmask,&gw,NULL,&ethernetif_init,&ethernet_input);//向网卡列表中添加一个网口
	
#if LWIP_DHCP			//如果使用DHCP的话
	lwipdev.dhcpstatus=0;	//DHCP标记为0
	dhcp_start(&lwip_netif);	//开启DHCP服务
#endif
	
	if(Netif_Init_Flag==NULL)return 3;//网卡添加失败 
	else//网口添加成功后,设置netif为默认值,并且打开netif网口
	{
		netif_set_default(&lwip_netif); //设置netif为默认网口
		netif_set_up(&lwip_netif);		//打开netif网口
	}
	return 0;//操作OK.
}   ```

###  void udp_demo_test(void)
**此函数的执行流程**
*下面执行过程都是递进的只有全部都成功才会显示准备完毕（res=1）否则res=0*
 1. 设置远端IP地址（按键调节）
 2. 创建udp控制块udppcb
 3. 初始化发送包
 4. 将远端IP地址打包成32位 ip_addr  rmtipaddr
 5. 将打包好的远端IP与端口号连接到udppcb
 6. 绑定本地IP地址与端口号
 7. 注册接受回调函数udp_demo_recv（此函数为我们接收到处理都在里面处理，后面详细讲解）
 8. 标记本地UDP准备完毕udp_demo_flag置一。
 9. while（）{用户自定义发送，接收}
 10. 发送完毕，关闭连接。
 11. 释放申请的动态内存 
 
 ~~~c
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
	///////////////////////////////////////////
	//根据用户需求自定义以下函数
	while(res==0)//连接成功
	{
		key=KEY_Scan(0);
		if(key==WKUP_PRES)break;
		if(key==KEY0_PRES)//KEY0按下了,发送数据
		{
			udp_demo_senddata(udppcb);
		}	
		if(udp_demo_flag&1<<6)//是否收到数据?
		{
			udp_demo_senddata(udppcb);//根据需求收到后给予服务器端回应
			elengine_wyx = 1;//电机转动标志（保证每次接受到转动一次）
			udp_demo_flag&=~(1<<6);//标记数据已经被处理了.
		} 
		lwip_periodic_handle();//轮询任务 维持LWIP内核
		delay_ms(2);
	}
	///////////////////////////////////////////
	udp_demo_connection_close(udppcb); 
	myfree(SRAMIN,tbuf);
} 

```
再来看看
###  void udp_demo_recv(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port)
*此函数的执行流程为*
1. 清空接收缓冲区
2. 判断数据缓冲区剩余的空间是否够用，如果不够用则只拷贝一部分直至缓冲区满。（因为是循环存储拷贝的，所以可能存在这种情况）。
3. 解析接收的数据包。
4. 根据解析后的数据包，使系统做出相应处理。
5. 记录远程IP地址并释放内存。
```c
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

```
和回调函数有关的三个函数：解析函数 前缀判断函数 处理函数。这三个函数较简单。
前缀判断函数：根据前面说的ZNJJ协议来判断有效数据部分
```c
u8 judge(void)//前缀判断函数
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
		if(position>=sendrecv.length) return 0;//说明没找到有效数据位
		else return position+4;//让position指向第一个有效数据位
}

void readrecv(void)
{
	u8 position;
	if(judge()==0);//判断数据是否符合ZNJJ 0 不符合  1  符合
	else//符合做相关处理
	{
		position=judge();//从该位提取控制数据   将接收数据存放在相应的标志位上
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

void Message_processing()
{		
	//printf("l:%s e:%s %s  \r\n",sendrecv.led,sendrecv.elengine,udp_demo_recvbuf);
	if(sendrecv.led[0]=='1')           	LED0=1;
	else 							 	LED0=0;
	if(sendrecv.led[1]=='1')          	LED1=1;
	else 							 	LED1=0;
	if(sendrecv.led[2]=='1')           	LED2=1;
	else 							 	LED2=0;
	//printf("wwwwwwwww%d  \r\n",sendrecv.elengine[0]);
	if(sendrecv.humi[0]==1){
		//printf("wwwwwwwww%d  \r\n",sendrecv.humi[0]);
		if(sendrecv.elengine[0]=='1'&&sendrecv.elengine[1]=='1'&&elengine_wyx)//控制发动机  elengine_wyx此变量用来控制在接收到一次数据时电机只转动一次，在接收时将他置为一
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

```
 ##  void udp_demo_senddata(struct udp_pcb *upcb)//发送数据函数  所发送的数据包在全局变量sendrecv结构体中。
 1. 温湿度传感器读取数据存放到temp，humi中
 2. 设置temp与humi的值到sendrecv结构体的data中
 3. 将设置好的数据放到要发送的udp_demo_sendbuf字符串里面
 4. 把udp_demo_sendbuf放到ptr中。
 5. 发送ptr数据。
```c
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
//设置温湿度到sendrecv的data结构体
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
//将sendrecv结构体数据放到udp_demo_sendbuf中
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

```




