#include "tcp_client.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include <string.h>
#include <stdio.h>
#include "netconf.h"
#include "udp_client.h"

#define PACKAGE_MAX              256

uint8_t ConnectCmd[] = "connect";


static err_t Tcp_RecFun(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t TCP_Client_Connected(void *arg, struct tcp_pcb *tpcb, err_t err);
static void tcp_err_callback(void *arg, err_t err);


//初始化TCP客户端
err_t TCP_Client_Attemp_Connect(tcp_infor_t *ts)
{
	err_t ret = ERR_OK;
    if(ts->state==S_IDLE)
	{
	    ts->tpcb = tcp_new();				//新建一个PCB	
		ts->tpcb->so_options |= SOF_KEEPALIVE;
		if(ts->tpcb==NULL)
			return ERR_BUF;
		ts->state = S_CONNECTING;		
	}
	
	ret = tcp_bind(ts->tpcb, IP_ADDR_ANY, ts->local_port);
//	if(ret!=ERR_OK)
//		return ret;
	ret = tcp_connect(ts->tpcb, &(ts->ip), ts->remote_port, TCP_Client_Connected);
	if(ret==ERR_OK)
	{
		tcp_arg(ts->tpcb, ts);  //回调函数参数传递
		tcp_err(ts->tpcb, tcp_err_callback);
	}
	ts->retry = 0;
	return ret;	
}

//客户端成功连接到远程主机时调用
static err_t TCP_Client_Connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
	tcp_infor_t *ps;
	
	ps = (tcp_infor_t*) arg;
	if(err==ERR_OK)
	{
		ps->retry = 0;
		ps->state = S_CONNECTED;
		tcp_arg(ps->tpcb, ps);  //回调函数参数传递
		tcp_recv(ps->tpcb, Tcp_RecFun);	//指定连接接收到新的数据之后将要调用的回调函数 
		tcp_write(ps->tpcb, ConnectCmd, sizeof(ConnectCmd), 1);
//		udp_client_Send(ps->, *addr, port, p->payload, p->len);
	}
	else
	{
		ps->state = S_CLOSED;
		tcp_arg(ps->tpcb, NULL);  			
		tcp_recv(ps->tpcb, NULL);
		tcp_close(ps->tpcb); 
		TCP_Client_Attemp_Connect(ps); //直接去链接，还是需要等待一段时间链接，需测试
//		set_timer4_countTime(TIMER_5000MS); 
	}
	
	return err;
}

/*
* 连接出错时，进行错误处理的函数
*/
static void tcp_err_callback(void *arg, err_t err)
{
	tcp_infor_t *ps;
	
	ps = (tcp_infor_t*) arg;
	if(err==ERR_OK)
		return;
	else
	{
		printf("[tcp_client]: ERR %d\r\n", err);
		ps->retry ++;
		if(ps->retry<5)
			TCP_Client_Attemp_Connect(ps); //直接去链接，还是需要等待一段时间链接，需要测试
			//set_timer4_countTime(TIMER_5000MS);
		else
		{
			tcp_client_close(ps);
		}
		
	}
}

void tcp_client_close( tcp_infor_t* ts)
{
	if(ts!=NULL)
	{
		ts->state = S_CLOSED;
		tcp_arg(ts->tpcb, NULL);  			
		tcp_recv(ts->tpcb, NULL);
		tcp_poll(ts->tpcb, NULL, 0); 
		tcp_close(ts->tpcb);
//		memset(ts, 0 , sizeof(tcp_infor_t));
	}
}

static err_t Tcp_RecFun(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	tcp_infor_t *ts;
	
	ts = (tcp_infor_t*)arg;
	if(err!= ERR_OK)
	{
		if(p!=NULL)
			pbuf_free(p);
	}
	else
	{
		ts->state = S_RECEIVE;
		ts->recv(tpcb,p,arg);
		ts->state = S_CLOSED;
		udp_client_Send(ts->pserver->upcb_server.upcb, ts->pserver->upcb_server.addr, ts->pserver->upcb_server.port, p->payload, p->len);
		pbuf_free(p);
	}
	tcp_client_close(ts);
	return err;
}

err_t TCP_Send(tcp_infor_t *es, uint8_t *msg, uint16_t len)
{
	struct pbuf *ptr;
	err_t wr_err = ERR_OK;
	
	if(es==NULL)
		return ERR_VAL;
	if(es->state!=S_CONNECTED)
		return ERR_CLSD;
	ptr = pbuf_alloc(PBUF_TRANSPORT, PACKAGE_MAX, PBUF_RAM);
	if(ptr==NULL)
		return ERR_BUF;
	ptr->payload = msg;
	ptr->len = ptr->tot_len = len;
	
	
	while(ptr->len<=tcp_sndbuf(es->tpcb)&&(wr_err == ERR_OK)&&(ptr!=NULL))
	{
		wr_err = tcp_write(es->tpcb, ptr->payload, ptr->len, 1); 
		if(wr_err==ERR_OK)
		{
			ptr = ptr->next;
			if(ptr!=NULL)
				pbuf_ref(ptr);
			
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
