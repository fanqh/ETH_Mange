#include "revogi.h"
#include "udp_client.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include <string.h>
#include <stdio.h>
//#include "ip_addr.h"

#define  REVOGI_ADV_REMOTE_PORT  8888
#define  REVOGI_ADV_LOCAL_PORT   8889
#define  BUFF_LEN_MAX            256

#define REVOGI_TCP_REMOTE_PORT   80
#define REVOGI_TCP_LOCAL_PORT    9999



revogi_infor_t revogi_infor;

static void revogi_rec_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
static err_t Trip_TCP_Client_Connected(void *arg, struct tcp_pcb *tpcb, err_t err);
static void tcp_err_callback(void *arg, err_t err);
static err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

//switch 初始化
err_t revogi_Init(device_infor_t *pDev)
{
	
	err_t ret = ERR_OK;
	
	revogi_infor.udp_adv_ip.addr = Device_Infor.pnetif->gw.addr | 0xff000000;
	revogi_infor.udp_pcb = udp_new();
	if(revogi_infor.udp_pcb==NULL)
		return ERR_BUF;
	pDev->udp_num++;
	revogi_infor.udp_remote_port = REVOGI_ADV_REMOTE_PORT;
	revogi_infor.udp_local_port = REVOGI_ADV_LOCAL_PORT;
	
	ret = udp_client_init(revogi_infor.udp_pcb, revogi_rec_callback, revogi_infor.udp_adv_ip, revogi_infor.udp_remote_port, revogi_infor.udp_local_port,&revogi_infor);
	return ret;
}
//switch udp 接收
static void revogi_rec_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
	uint8_t rec[BUFF_LEN_MAX];
	uint8_t i;
	struct ip_addr ip_addr;
	uint8_t *ptr;
	revogi_infor_t *pSw;
	
	pSw = (revogi_infor_t*)arg;
	printf("[Revogi]: broadcast ip: %X\r\n", (uint32_t)addr->addr);
	memset(rec,0,BUFF_LEN_MAX);
	memcpy(rec, p->payload,p->len);
	revogi_infor.tcp_ip.addr = addr->addr;
//	if((NumofStr(rec, '.')==3)&&(NumofStr(rec, ',')==5))
//	{
//		if(inet_aton(rec, (struct in_addr*)&ip_addr)==0)
//			return;
//		if(ip_addr.addr!=addr->addr)
//			return;
//		revogi_infor.tcp_ip.addr = ip_addr.addr;
//		
//		ptr = strstr((char*)rec, ",");
//		if(ptr==NULL)
//			return;
//		memcpy(revogi_infor.mac, ptr+1,12);
//		ptr = strstr((char*)(ptr+1),",");
//		if(ptr==NULL)
//			return;
//		memcpy(revogi_infor.sn,ptr+1,9);
//		ptr = strstr((char*)(ptr+10),",");
//		if(ptr==NULL)
//			return;
//		revogi_infor.is_online =(bool) *(ptr+1);
//		revogi_infor.state =(bool) *(ptr+3);
//	}
	
	
	
	printf("[Revogi]: ");
	for(i=0;i<p->len;i++)
	{
		printf("%c",rec[i] );
	}
	printf("\r\n");

  pbuf_free(p);
}

//switch udp 发送
err_t revogi_find_udp_Send(uint8_t *p, uint16_t len)
{
		err_t ret = ERR_OK;
	
		ret = udp_client_Send(revogi_infor.udp_pcb, revogi_infor.udp_adv_ip, revogi_infor.udp_remote_port, p, len);
		return ret;
}



//初始化TCP客户端
err_t PowerTrip_TCP_Client_Attemp_Connect(revogi_infor_t  *ps)
{
	err_t ret = ERR_OK;
	
//	if(ps->sw_state==UNUSE)
//	{ 
		ps->tcp_pcb = tcp_new();				//新建一个PCB
		ps->tcp_pcb->so_options |= SOF_KEEPALIVE;
		if(ps->tcp_pcb==NULL)
			return ERR_BUF;
//		ps->sw_state = DISCONNECT;
//	}
	ret = tcp_bind(ps->tcp_pcb, IP_ADDR_ANY, REVOGI_TCP_LOCAL_PORT);
	if(ret!=ERR_OK)
		return ret;
  ret = tcp_connect(ps->tcp_pcb, &ps->tcp_ip, REVOGI_TCP_REMOTE_PORT, Trip_TCP_Client_Connected);
	if(ret==ERR_OK)
	{
		tcp_arg(ps->tcp_pcb, ps);  //回调函数参数传递
		tcp_err(ps->tcp_pcb, tcp_err_callback);
	}
	return ret;
}

//客户端成功连接到远程主机时调用
static err_t Trip_TCP_Client_Connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
	revogi_infor_t  *pes;
	
	pes = (revogi_infor_t*)arg;
	if(err == ERR_OK)
	{
//		pes->sw_state = CONNECTED;		
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
void PowerTriptcp_client_close( revogi_infor_t* ts)
{
	if(ts!=NULL)
	{
//		ts->sw_state = DISCONNECT;
		tcp_arg(ts->tcp_pcb, NULL);  			
		tcp_recv(ts->tcp_pcb, NULL);
	//	tcp_poll(ts->tcp_pcb, NULL, 0); 
		tcp_close(ts->tcp_pcb);
		memset(ts, 0 , sizeof(revogi_infor_t));
	}
}
/*
* 连接出错时，进行错误处理的函数
*/
static void tcp_err_callback(void *arg, err_t err)
{
	revogi_infor_t  *es;
	
	es = (revogi_infor_t*)arg;
	if(err==ERR_OK)
		return ;
	else
	{
		printf("[SW]: %d\r\n", err);
//		es->connect_count++;
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

static err_t tcp_client_send(revogi_infor_t *ps)
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
err_t PowerTrip_TCP_Send(revogi_infor_t *es, uint8_t *msg, uint16_t len)
{
//	if(es->sw_state!=CONNECTED)
//		return ERR_CONN;
	if(es->p==NULL)
		es->p = pbuf_alloc(PBUF_TRANSPORT, BUFF_LEN_MAX, PBUF_RAM); 
	if(es->p==NULL)
		return ERR_BUF;
	es->p->payload = msg;
	es->p->len = es->p->tot_len = len;
	return tcp_client_send(es);
}

static err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	uint8_t rec[256];
	revogi_infor_t  *es;
	es = (revogi_infor_t*)arg;
	
	printf("[PowerTrip]: tcp received\r\n");
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
//		es->sw_state = DISCONNECT;
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
		memset(rec, 0, 256);	
		memcpy(rec, (uint8_t*)(p->payload), p->tot_len);
		pbuf_free(p); 
	}
	return err;
}



