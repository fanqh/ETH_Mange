#include "main.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include <string.h>
#include <stdio.h>
#include "smart_switch.h"
#include "netconf.h"
#include "udp_client.h"
#include "device_server.h"

#define SWTICH_ADV_PORT   			 48899
#define SWITCH_UPD_LOCAL_PORT  	 48899

#define SWITCH_TCP_PORT          8899
#define SWITCH_TCP_LOCAL_PORT    9008

#define PACKAGE_MAX              256

smart_switch_infor_t switch_infor;
struct ip_addr adv_ip;
uint8_t udp_init =0;
struct tcp_pcb *TcpPCB;

//const uint8_t SwitchAdvCMD[] = "YZ-RECOSCAN";

static void switch_rec_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
static err_t Switch_TCP_Client_Connected(void *arg, struct tcp_pcb *tpcb, err_t err);
static err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void tcp_err_callback(void *arg, err_t err);


//switch 初始化
err_t Switch_Init(device_infor_t *pDev)
{
	
	err_t ret = ERR_OK;
	
	switch_infor.udp_adv_ip.addr = Device_Infor.pnetif->gw.addr | 0xff000000;
	switch_infor.udp_pcb = udp_new();
	if(switch_infor.udp_pcb==NULL)
		return ERR_BUF;
	pDev->udp_num++;
	switch_infor.pdev = pDev;
	switch_infor.udp_remote_port = SWTICH_ADV_PORT;
	switch_infor.udp_local_port = SWITCH_UPD_LOCAL_PORT;
	
	
	ret = udp_client_init(switch_infor.udp_pcb, switch_rec_callback, switch_infor.udp_adv_ip, switch_infor.udp_remote_port, switch_infor.udp_local_port, &switch_infor);
	return ret;
}
//switch udp 接收
static void switch_rec_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
	uint8_t rec[PACKAGE_MAX];
	uint8_t i;
	struct ip_addr ip_addr;
	uint8_t *ptr;
	smart_switch_infor_t *pSw;
	Dev_Server_infor_t* pserver;
	
	pSw = (smart_switch_infor_t*)arg;
	pserver = pSw->pdev->p;
	printf("[sw]: broadcast ip: %X\r\n", (uint32_t)addr->addr);
	memset(rec,0,PACKAGE_MAX);
	memcpy(rec, p->payload,p->len);
	if((NumofStr(rec, '.')==3)&&(NumofStr(rec, ',')==5))
	{
		if(inet_aton(rec, (struct in_addr*)&ip_addr)==0)
			return;
		if(ip_addr.addr!=addr->addr)
			return;
		switch_infor.tcp_ip.addr = ip_addr.addr;
		
		ptr = strstr((char*)rec, ",");
		if(ptr==NULL)
			return;
		memcpy(switch_infor.mac, ptr+1,12);
		ptr = strstr((char*)(ptr+1),",");
		if(ptr==NULL)
			return;
		memcpy(switch_infor.sn,ptr+1,9);
		ptr = strstr((char*)(ptr+10),",");
		if(ptr==NULL)
			return;
		switch_infor.is_online =(bool) *(ptr+1);
		switch_infor.state =(bool) *(ptr+3);
	}
	
	
	udp_client_Send(pserver->upcb_server.upcb, pserver->upcb_server.addr, pserver->upcb_server.port, p->payload, p->len);
	printf("[sw]: ");
	for(i=0;i<p->len;i++)
	{
		printf(" %X",rec[i] );
	}
	printf("\r\n");

  pbuf_free(p);
}

//switch udp 发送
err_t switch_udp_Send(uint8_t *p, uint16_t len)
{
		err_t ret = ERR_OK;
	
		ret = udp_client_Send(switch_infor.udp_pcb, switch_infor.udp_adv_ip, switch_infor.udp_remote_port, p, len);
		return ret;
}



//初始化TCP客户端
err_t Switch_TCP_Client_Attemp_Connect(smart_switch_infor_t  *ps)
{
	err_t ret = ERR_OK;
	
	if(ps->sw_state==UNUSE)
	{ 
		ps->tcp_pcb = tcp_new();				//新建一个PCB
		ps->tcp_pcb->so_options |= SOF_KEEPALIVE;
		if(ps->tcp_pcb==NULL)
			return ERR_BUF;
		ps->sw_state = DISCONNECT;
	}
	ret = tcp_bind(ps->tcp_pcb, IP_ADDR_ANY, SWITCH_TCP_LOCAL_PORT);
	if(ret!=ERR_OK)
		return ret;
  ret = tcp_connect(ps->tcp_pcb, &ps->tcp_ip, SWITCH_TCP_PORT, Switch_TCP_Client_Connected);
	if(ret==ERR_OK)
	{
		tcp_arg(ps->tcp_pcb, ps);  //回调函数参数传递
		tcp_err(ps->tcp_pcb, tcp_err_callback);
	}
	return ret;
}

//客户端成功连接到远程主机时调用
static err_t Switch_TCP_Client_Connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
	smart_switch_infor_t  *pes;
	
	pes = (smart_switch_infor_t*)arg;
	if(err == ERR_OK)
	{
		pes->sw_state = CONNECTED;		
		tcp_arg(pes->tcp_pcb, pes);  //回调函数参数传递		
		tcp_recv(pes->tcp_pcb, tcp_client_recv);	//指定连接接收到新的数据之后将要调用的回调函数 
		printf("[SW]: connected\r\n");
	}
//	else
//	{
//		pes->sw_state = DISCONNECT;
//		
//		tcp_arg(pes->tcp_pcb, NULL);  			
//		tcp_recv(pes->tcp_pcb, NULL);
//		tcp_close(pes->tcp_pcb); 
////		set_timer4_countTime(TIMER_5000MS);  
//	}
	
	return err;
}

//关闭TCP链接
void tcp_client_close( smart_switch_infor_t* ts)
{
	if(ts!=NULL)
	{
		ts->sw_state = DISCONNECT;
		tcp_arg(ts->tcp_pcb, NULL);  			
		tcp_recv(ts->tcp_pcb, NULL);
	//	tcp_poll(ts->tcp_pcb, NULL, 0); 
		tcp_close(ts->tcp_pcb);
		memset(ts, 0 , sizeof(smart_switch_infor_t));
	}
}
/*
* 连接出错时，进行错误处理的函数
*/
static void tcp_err_callback(void *arg, err_t err)
{
	smart_switch_infor_t  *es;
	
	es = (smart_switch_infor_t*)arg;
	if(err==ERR_OK)
		return ;
	else
	{
		printf("[SW]: %d\r\n", err);
		es->connect_count++;
		tcp_client_close(es);
//		if(es->connect_count<3)
//			set_timer4_countTime(TIMER_5000MS); 
//		else001
//		{
//			connet_flag = 4;  pbuf_take pbuf_ref
//			set_timer4_countTime(TIMER_10000MS);tcp_sndbuf pbuf_take
//		}	
	}
}

static err_t tcp_client_send(smart_switch_infor_t *ps)
{
	err_t wr_err = ERR_OK;
	struct pbuf *ptr;
	
	while(ps->p->len<=tcp_sndbuf(ps->tcp_pcb)&&(wr_err == ERR_OK)&&(ps->p!=NULL))
	{
		ptr = ps->p;
		wr_err = tcp_write(ps->tcp_pcb, ptr->payload, ptr->len, 1); 
		if(wr_err==ERR_OK)
		{
			ps->p = ptr->next;
			if(ps->p!=NULL)
				pbuf_ref(ps->p);
			
		}
		else 
		{
			pbuf_free(ptr);
			return wr_err;
		}
	}
	pbuf_free(ptr);
	
	return wr_err;
	
}
err_t Switch_TCP_Send(smart_switch_infor_t *es, uint8_t *msg, uint16_t len)
{
//	if(es->sw_state!=CONNECTED)
//		return ERR_CONN;
	if(es->p==NULL)
		es->p = pbuf_alloc(PBUF_TRANSPORT, PACKAGE_MAX, PBUF_RAM); 
	if(es->p==NULL)
		return ERR_BUF;
	es->p->payload = msg;
	es->p->len = es->p->tot_len = len;
	return tcp_client_send(es);
}

static err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	uint8_t rec[256];
	smart_switch_infor_t  *es;
	es = (smart_switch_infor_t*)arg;
	
	printf("[SW]: tcp received\r\n");
//	
//	if(err!=ERR_OK)
//	{
//		if(p!=NULL)
//			pbuf_free(p);
//		return err;
//	}
//	if(es->sw_state==CONNECTED)
//	{
//		tcp_recved(tpcb, p->tot_len);   //读取数据
////		memset(rec, 0, 256);
//		if(p->len>=256)
//			p->len = 256;
//		memcpy(rec, (uint8_t*)(p->payload), p->len);
//	}
//	else
//	{
//		tcp_recved(tpcb, p->tot_len);   //读取数据
//	}
//	pbuf_free(p);  
	if(p==NULL)
	{
		es->sw_state = DISCONNECT;
		tcp_client_close(es);
	}
	else if(err!=ERR_OK)
	{
		if(p!=NULL)
				pbuf_free(p);
		return err;
	}
	else
	{
		if(p->tot_len >=256)
			p->tot_len = 256;
		memcpy(rec, (uint8_t*)(p->payload), p->len);	
		memcpy(rec, (uint8_t*)(p->payload), p->tot_len);
		pbuf_free(p); 
	}
	return err;
}


uint8_t CompareMac(uint8_t *pmac1, uint8_t *pmac2)
{
	uint8_t i;
	
	for(i=0; i<6; i++)
	{
		if(*pmac1++ != *pmac2++)
			return 0;
	}
	return 1;
}

int NumofStr(char*str, char c)
{
	char *p;
	int num = 0;
	
	p = str;
	while(*p!='\0')
	{
		if(*p==c)
			num++;
		++p;
	}
	return num;
}
