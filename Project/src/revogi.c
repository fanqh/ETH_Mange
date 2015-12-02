#include "revogi.h"
#include "udp_client.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include <string.h>
#include <stdio.h>
#include "tcp_client.h"

#define  REVOGI_ADV_REMOTE_PORT  8888
#define  REVOGI_ADV_LOCAL_PORT   8889
#define  BUFF_LEN_MAX            256

#define REVOGI_TCP_REMOTE_PORT   80
#define REVOGI_TCP_LOCAL_PORT    9999




static void revogi_udp_rec_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
err_t PowerTrip_Tcp_Rec(struct pbuf *p, void *arg, err_t err);
static void  PowerTrip_connectedf(void *arg);
static void PowerTrip_connecterrf(void *arg);
static void PowerTrip_connectclose(void *arg);


revogi_infor_t revogi_infor=
{
	0,
	{{S_IDLE,0,0,REVOGI_ADV_REMOTE_PORT,REVOGI_ADV_LOCAL_PORT,revogi_udp_rec_callback}, //udp
	{0,S_IDLE,0,0,REVOGI_TCP_LOCAL_PORT,REVOGI_TCP_REMOTE_PORT,PowerTrip_Tcp_Rec,PowerTrip_connectedf,PowerTrip_connecterrf,PowerTrip_connectclose,0}},
	0,
	{{0},{0},{0},{0},FALSE,FALSE,0},
	0
};

//switch 初始化
err_t revogi_Init(device_infor_t *pDev)
{
	
	err_t ret = ERR_OK;
	
	revogi_infor.adv_ip.addr = Device_Infor.pnetif->gw.addr | 0xff000000;
	revogi_infor.net.udp.upcb = udp_new();
	if(revogi_infor.net.udp.upcb==NULL)
		return ERR_BUF;
	revogi_infor.pdev = pDev;
	pDev->udp_num++;

	ret = udp_client_init(&revogi_infor.net.udp, &revogi_infor);
	return ret;
}
//XX:XX:XX:XX:XX:XX
static int extractMac(char *des,char *src)
{
	char *p;
	char d[18];
	uint8_t i;
	
	p = des;
	memset(d, 0, 18);
	memcpy(d,src,17);
	if(NumofStr(d,':')!=5)
		return 0;
	for(i=0;i<6;i++)
	{
		memcpy(p+i*2, d+3*i,2);
	}
	return 1;
}

//switch udp 接收
static void revogi_udp_rec_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
	char rec[BUFF_LEN_MAX];
	uint8_t i;
	char *ptr;
	revogi_infor_t *pSw;
	Dev_Server_infor_t* pserver;
	
	pSw = (revogi_infor_t*)arg;
	pserver = pSw->pdev->server;
	printf("[Revogi]: broadcast ip: %X\r\n", (uint32_t)addr->addr);
	memset(rec,0,BUFF_LEN_MAX);
	memcpy(rec, p->payload,p->len);
	
	if((strstr(rec, "response"))&&(strstr(rec, "name")) &&(strstr(rec, "mac")) &&(strstr(rec, "protect")))//&& (strstr(rec,":{\"sn\":\"")) && (strstr(rec,"}}"))
	{
		revogi_infor.net.tcp.tip.addr = addr->addr;
		revogi_infor.net.udp.uip.addr = addr->addr;
		if((ptr = strstr(rec, "\"sn\":"))!=NULL)
			memcpy(pSw->comm.sn,ptr+6,16);
		if((ptr=strstr(ptr,"\"ver\":\""))!=NULL)
			memcpy(pSw->comm.dver, ptr+7, 4);
		if((ptr=strstr(ptr,"\"register\":"))!=NULL)
			pSw->comm.regist = ((*(ptr + 11)=='1')? TRUE:FALSE);
		if((ptr=strstr(ptr,"\"nver\":\""))!=NULL)
			memcpy(pSw->comm.sver, ptr+8, 4);
		if((ptr=strstr(ptr,"\"protect\":"))!=NULL)
			pSw->comm.protect = ((*(ptr + 10)=='1')? TRUE:FALSE);
		if((ptr=strstr(rec,"\"mac\":\""))!=NULL)
		{
			ptr = ptr + 7;
			extractMac(pSw->comm.mac, ptr);
		}
		
	}
	udp_client_Send(&pserver->sudp, pserver->sudp.uip, p->payload, p->len);
	printf("[Revogi]: ");
	for(i=0;i<p->len;i++)
	{
		printf("%c",rec[i] );
	}
	printf("\r\n");

  pbuf_free(p);
}

err_t revogi_udp_Send(struct ip_addr addr, uint8_t *p, uint16_t len)
{
		err_t ret = ERR_OK;
	
		ret = udp_client_Send(&revogi_infor.net.udp, addr, p, len);
		return ret;
}



static void  PowerTrip_connectedf(void *arg)
{
//	tcp_struct_t  *ts;
	struct pbuf *p;
	
//	ts = (tcp_struct_t*)arg;
	p = pbuf_alloc(PBUF_TRANSPORT, PACKAGE_MAX, PBUF_RAM);
	
	memcpy(p->payload,connectedcmd,sizeof(connectedcmd));
	p->tot_len = p->len = sizeof(connectedcmd);
	udp_client_Send(&revogi_infor.pdev->server->sudp, revogi_infor.pdev->server->sudp.uip, p->payload, p->len);
	pbuf_free(p);
}	
static void PowerTrip_connecterrf(void *arg)
{
//	tcp_struct_t  *ts;
	struct pbuf *p;
	
//	ts = (tcp_struct_t*)arg;
	p = pbuf_alloc(PBUF_TRANSPORT, PACKAGE_MAX, PBUF_RAM);
	
	memcpy(p->payload,connecterrcmd,sizeof(connecterrcmd));
	p->tot_len = p->len = sizeof(connecterrcmd);
	udp_client_Send(&revogi_infor.pdev->server->sudp, revogi_infor.pdev->server->sudp.uip, p->payload, p->len);
	pbuf_free(p);
}
static void PowerTrip_connectclose(void *arg)
{
//	tcp_struct_t  *ts;
	struct pbuf *p;
	
//	ts = (tcp_struct_t*)arg;
	p = pbuf_alloc(PBUF_TRANSPORT, PACKAGE_MAX, PBUF_RAM);
	
	memcpy(p->payload,closecmd,sizeof(closecmd));
	p->tot_len = p->len = sizeof(closecmd);
	udp_client_Send(&revogi_infor.pdev->server->sudp, revogi_infor.pdev->server->sudp.uip, p->payload, p->len);
	pbuf_free(p);
}




//初始化TCP客户端
err_t PowerTrip_TCP_Client_Attemp_Connect(revogi_infor_t  *ps)
{
	return TCP_Client_Attemp_Connect(&ps->net.tcp);
}

//关闭TCP链接
void PowerTrip_TCP_Client_Close( revogi_infor_t* ss)
{	
	tcp_client_close(&ss->net.tcp);
}

err_t PowerTrip_TCP_Send(revogi_infor_t *es, uint8_t *msg, uint16_t len)
{
	if(es==NULL)
		return ERR_VAL;
	if(&es->net.tcp==NULL)
		return ERR_VAL;
	return TCP_Send(&es->net.tcp, msg, len);
}
err_t PowerTrip_Tcp_Rec(struct pbuf *p, void *arg, err_t err)
{
	uint8_t rec[256];
	tcp_struct_t  *es;
	es = (tcp_struct_t*)arg;
	
	printf("[SW]: tcp received\r\n");
	if(p==NULL||(err!=ERR_OK))
	{
		return ERR_BUF;
	}
	else
	{
		if(p->tot_len >=256)
			p->tot_len = 256;
		memcpy(rec, (uint8_t*)(p->payload), p->len);	
		udp_client_Send(&revogi_infor.pdev->server->sudp, revogi_infor.pdev->server->sudp.uip, p->payload, p->len);
		pbuf_free(p); 
	}
	return err;
}

