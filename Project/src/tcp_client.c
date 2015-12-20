#include "tcp_client.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include <string.h>
#include <stdio.h>
#include "netconf.h"
#include "udp_client.h"

static err_t Tcp_RecFun(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t TCP_Client_Connected(void *arg, struct tcp_pcb *tpcb, err_t err);
static void tcp_err_callback(void *arg, err_t err);



//初始化TCP客户端
err_t TCP_Client_Attemp_Connect(tcp_struct_t *ts)
{
	err_t ret = ERR_OK;
	
//	if(ts->tstate==S_CONNECTED)
//		ts->connectedf(ts);
    if(ts->tstate==S_IDLE)
	{
	    ts->tpcb = tcp_new();				//新建一个PCB	
		ts->tpcb->so_options |= SOF_KEEPALIVE;
		if(ts->tpcb==NULL)
			return ERR_BUF;
		ts->tstate = S_CONNECTING;		
	}
	ret = tcp_bind(ts->tpcb, IP_ADDR_ANY, ts->tlocal_port);
//	if(ret!=ERR_OK)
//		return ret;
	
	ret = tcp_connect(ts->tpcb, &(ts->tip), ts->tremote_port, TCP_Client_Connected);
	if(ret==ERR_OK)
	{
		tcp_arg(ts->tpcb, ts);  //回调函数参数传递
		tcp_err(ts->tpcb, tcp_err_callback);
	}
	else if(ret==ERR_ISCONN)
	{
//		tcp_client_close(ts);
//		tcp_connect(ts->tpcb, &(ts->tip), ts->tremote_port, TCP_Client_Connected);
	}
	ts->retry = 0;
	return ret;	
}
//客户端成功连接到远程主机时调用
static err_t TCP_Client_Connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
	tcp_struct_t *ps;
	
	ps = (tcp_struct_t*) arg;
	if(err==ERR_OK)
	{		
		ps->retry = 0;
		ps->tstate = S_CONNECTED;
		tcp_arg(ps->tpcb, ps);  //回调函数参数传递
		tcp_recv(ps->tpcb, Tcp_RecFun);	//指定连接接收到新的数据之后将要调用的回调函数
		if(ps->connectedf!=NULL)
			ps->connectedf(ps);
	}
	else
	{
		ps->tstate = S_CONNECTING;
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
	tcp_struct_t *ps;
	
	ps = (tcp_struct_t*) arg;
	if(err==ERR_OK)
		return;
	else
	{
		printf("[tcp_client]: ERR %d\r\n", err);
//		ps->retry ++;
//		ps->tstate = S_CLOSED;
//		if(ps->retry<1)
//			TCP_Client_Attemp_Connect(ps); //直接去链接，还是需要等待一段时间链接，需要测试
//			//set_timer4_countTime(TIMER_5000MS);
//		else
		{
			
			if(ps->connecterrf!=NULL)
				ps->connecterrf(ps);
//			tcp_client_close(ps);
			//ps->tstate = S_CLOSED;
		}
	}
}

void tcp_client_close( tcp_struct_t* ts)
{
	if(ts!=NULL)
	{
		ts->tstate = S_IDLE;
		tcp_arg(ts->tpcb, NULL);  			
		tcp_recv(ts->tpcb, NULL);
		tcp_poll(ts->tpcb, NULL, 0); 
		tcp_abort(ts->tpcb);
	}
	if(ts->connectclose!=NULL)
		ts->connectclose(ts);
}

static err_t Tcp_RecFun(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	tcp_struct_t *ts;
	
	ts = (tcp_struct_t*)arg;
	if(err!= ERR_OK)
	{
		if(p!=NULL)
			pbuf_free(p);
	}
	else
	{
//		ts->tstate = S_RECEIVE;
		ts->recv(p, arg, err);
//		udp_client_Send(ts->pserver->upcb_server.upcb, ts->pserver->upcb_server.addr, ts->pserver->upcb_server.port, p->payload, p->len);
		pbuf_free(p);
	}
	if(ts->keepconnect!=1)
	{
		ts->tstate = S_CLOSED;
		tcp_client_close(ts);
	}
	return err;
}

err_t TCP_Send(tcp_struct_t *es, uint8_t *msg, uint16_t len)
{
	struct pbuf *ptr;
	err_t wr_err = ERR_OK;
	
	if(es==NULL)
		return ERR_VAL;
	if(es->tstate!=S_CONNECTED)
		return ERR_CLSD;
	ptr = pbuf_alloc(PBUF_TRANSPORT, PACKAGE_MAX, PBUF_RAM);
	if(ptr==NULL)
		return ERR_BUF;
	ptr->payload = msg;
	ptr->len = ptr->tot_len = len;
	
	//while(ptr->len<=tcp_sndbuf(es->tpcb)&&(wr_err == ERR_OK)&&(ptr!=NULL))
	{
		wr_err = tcp_write(es->tpcb, ptr->payload, ptr->len, 1); 
//		if(wr_err==ERR_OK)
//		{
//			ptr = ptr->next;
//			if(ptr!=NULL)
//				pbuf_ref(ptr);
//			
//		}
//		else 
//		{
//			pbuf_free(ptr);
//			return wr_err;
//		}
	}
	pbuf_free(ptr);
	return wr_err;
}
