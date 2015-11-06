#include "main.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include <string.h>
#include <stdio.h>
#include "smart_switch.h"
#include "netconf.h"

#define SWTICH_ADV_PORT   		 48899
#define SWITCH_UPD_LOCAL_PORT  	 48899
#define SWITCH_TCP_PORT          8899

smart_switch_infor_t switch_infor;
struct ip_addr adv_ip;


const uint8_t SwitchAdvCMD[] = "YZ-RECOSCAN";

static void switch_rec_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
static err_t tcp_client_connect(void *arg, struct tcp_pcb *tpcb, err_t err);
static err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void tcp_err_callback(void *arg, err_t err);

err_t Switch_Init(void)
{
	
	err_t ret = ERR_OK;
	
	adv_ip.addr = Device_Infor.gw.addr | 0xff000000;

	switch_infor.udp_pcb = udp_new();
	if(switch_infor.udp_pcb==NULL)
		return ERR_BUF;
	switch_infor.udp_pcb->local_port = SWITCH_UPD_LOCAL_PORT;
	ret = udp_bind(switch_infor.udp_pcb, IP_ADDR_ANY, SWITCH_UPD_LOCAL_PORT);
	if(ret!=ERR_OK)
	{
		udp_remove(switch_infor.udp_pcb);
		
		return ret;
	}
	ret = udp_connect(switch_infor.udp_pcb, &adv_ip, SWTICH_ADV_PORT);
	if(ret!=ERR_OK)
	{
		udp_remove(switch_infor.udp_pcb);
		return ret;
	}
	udp_recv(switch_infor.udp_pcb, switch_rec_callback, NULL);
	return ret;
}

static void switch_rec_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
	uint8_t rec[256];
	uint8_t i;
	
	printf("[switch]: broadcast ip: %X\r\n", (uint32_t)addr->addr);
	memcpy(rec, p->payload,p->len);
	printf("[switch]: ");
	for(i=0;i<p->len;i++)
	{
		printf(" %X",rec[i] );
	}
	printf("\r\n");

  pbuf_free(p);
}


err_t switch_udp_Send(uint8_t *p, uint16_t len)
{
		struct pbuf *pSend;
		err_t ret = ERR_OK;
	
		pSend = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
		if(pSend==NULL)
			return ERR_BUF;
		pSend->payload = p;
		pSend->tot_len = pSend->len = len;
		
		ret = udp_connect(switch_infor.udp_pcb, &adv_ip, SWTICH_ADV_PORT);
		if(ret!=ERR_OK)
		{
			udp_remove(switch_infor.udp_pcb);
			return ret;
		}
		ret = udp_send(switch_infor.udp_pcb, pSend); 
		if(ret!=ERR_OK)
			printf("send failed\r\n");
		udp_disconnect(switch_infor.udp_pcb);
		
		pbuf_free(pSend);
		return ret;
}


//初始化TCP客户端
err_t Switch_TCP_Client_Attemp_Connect(smart_switch_infor_t  *ps)
{
	err_t ret = ERR_OK;
	
	ps->is_connect = 0;
	ps->tcp_pcb = tcp_new();				//新建一个PCB
	if(ps->tcp_pcb==NULL)
		return ERR_BUF;
    ret = tcp_connect(ps->tcp_pcb, &ps->tcp_ip, SWITCH_TCP_PORT, tcp_client_connected);
//	tcp_recv(switch_infor.tcp_pcb, tcp_client_recv);	//指定连接接收到新的数据之后将要调用的回调函数 
	tcp_arg(ps->tcp_pcb, ps);  //回调函数参数传递
	tcp_err(ps->tcp_pcb, tcp_err_callback);
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
		tcp_recv(switch_infor.tcp_pcb, tcp_client_recv);	//指定连接接收到新的数据之后将要调用的回调函数 
	}
	else
	{
		pes->sw_state = DISCONNECT;
		
		tcp_arg(pes->tcp_pcb, NULL);  			
		tcp_recv(pes->tcp_pcb, NULL);
		tcp_close(pes->tcp_pcb); 
//		set_timer4_countTime(TIMER_5000MS);  
	}
	
	return err_t;
}

//关闭连接
void tcp_client_close(struct tcp_pcb *tpcb, struct tcp_client_state* ts)
{

	tcp_arg(tcp_client_pcb, NULL);  			
	tcp_recv(tcp_client_pcb, NULL);
	tcp_poll(tcp_client_pcb, NULL, 0); 
	if(ts!=NULL){
		mem_free(ts);
	}
	tcp_close(tpcb);
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
//		if(es->connect_count<3)
//			set_timer4_countTime(TIMER_5000MS); 
//		else
//		{
//			connet_flag = 4;  
//			set_timer4_countTime(TIMER_10000MS);tcp_sndbuf pbuf_take
//		}	
	}
}


err_t Switch_TCP_Send(uint8_t *p, uint16_t len, smart_switch_infor_t *es)
{
	
}

static err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	smart_switch_infor_t  *es;
	es = (smart_switch_infor_t*)arg;
	
	if(err!=ERR_OK)
	{
		if(p!=NULL)
			free(p);
		return err;
	}
	if(es->sw_state==CONNECT)
	{
		tcp_recved(tpcb, p->tot_len);   //读取数据
	}
	else
	{
	}
	pbuf_free(p);  
	
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
