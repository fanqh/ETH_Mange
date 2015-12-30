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
#include "tcp_client.h"
#include "device_config.h"

#define SWTICH_ADV_PORT   			 48899
#define SWITCH_UPD_LOCAL_PORT  	 	 48899

#define SWITCH_TCP_PORT              8899
#define SWITCH_TCP_LOCAL_PORT        9008


static void  connectedf(void *arg);
static void connecterrf(void *arg);
static void connectclose(void *arg);
static void switch_rec_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
err_t Switch_Tcp_Rec(struct pbuf *tpcb,void *arg, err_t err);


struct ip_addr adv_ip;
uint8_t udp_init =0;
struct tcp_pcb *TcpPCB;

uint8_t SwitchAdvCMD[] = "YZ-RECOSCAN";

smart_switch_infor_t switch_infor=
{
	0,
	{{S_IDLE, 0,0,SWTICH_ADV_PORT,SWITCH_UPD_LOCAL_PORT,switch_rec_callback},//udp
	{0,S_IDLE,{0},0,SWITCH_TCP_LOCAL_PORT,SWITCH_TCP_PORT,Switch_Tcp_Rec,connectedf,connecterrf,connectclose}},//tcp
	{{0}, //mac
	{0},  //sn
	FALSE,//online
	FALSE},//state
	{0},     //adv_ip
	0     //pdev
};


//const uint8_t SwitchAdvCMD[] = "YZ-RECOSCAN";

static void switch_rec_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);


//switch 初始化
err_t Switch_Init(device_infor_t *pDev)
{
	
	err_t ret = ERR_OK;
	
	switch_infor.adv_ip.addr = Device_Infor.pnetif->gw.addr | 0xff000000;
	switch_infor.netlink.udp.upcb = udp_new();
	if(switch_infor.netlink.udp.upcb==NULL)
		return ERR_BUF;
	pDev->udp_num++;
	switch_infor.pdev = pDev;
	ret = udp_client_init(&switch_infor.netlink.udp, &switch_infor);
	Switch_find();
	return ret;
}

uint8_t AsciiToDisgital(char a)
{
	uint8_t n = 0xff;
	
	if((a>=0x30)&&(a<=0x39))
		n = a-0x30;
	else if((a>=0x41)&&(a<=0x46))
		n = a - 0x41 + 10;
	else if((a>=0x61)&&(a<=0x66))
		n = a - 0x61 + 10;
	return n;
}

int MacAsciiToInt(uint8_t *cp, uint8_t *mac)
{
	uint8_t i, a ,b;
	
	
	for(i=0; i<6; i++)
	{
		a = AsciiToDisgital(*(cp+2*i));
		b = AsciiToDisgital(*(cp+2*i+1));
		if((a==0xff) || (b ==0xff))
			return 0;
		else
			*(mac+i) = a * 10 + b;
	}
	return 1;
}

//switch udp 接收
static void switch_rec_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
	uint8_t rec[PACKAGE_MAX];
	uint8_t i;
	struct ip_addr ip_addr;
	char *ptr;
	smart_switch_infor_t *pSw;
	Dev_Server_infor_t* pserver;
	uint8_t cp[12],mac[6];
	Device_t dev;
	
	pSw = (smart_switch_infor_t*)arg;
	pserver = pSw->pdev->server;
	
	printf("[sw]: broadcast ip: %X\r\n", (uint32_t)addr->addr);
	memset(rec,0,PACKAGE_MAX);
	memcpy(rec, p->payload, p->len);
#if 1
	if((NumofStr(rec, '.')==3)&&(NumofStr(rec, ',')==5))
	{
		if(inet_aton(rec, (struct in_addr*)&ip_addr)==0)
			return;
		if(ip_addr.addr!=addr->addr)
			return;
		pSw->netlink.udp.uip.addr = ip_addr.addr;
		pSw->netlink.tcp.tip.addr = ip_addr.addr;
		
		ptr = strstr((char*)rec, ",");
		if(ptr==NULL)
			return;
		memcpy(cp, ptr+1,12);
		ptr = strstr((char*)(ptr+1),",");
		if(ptr==NULL)
			return;
		memcpy(pSw->smartplug_comm.sn,ptr+1,9);
		ptr = strstr((char*)(ptr+10),",");
		if(ptr==NULL)
			return;
		pSw->smartplug_comm.is_online =(bool) *(ptr+1);
		pSw->smartplug_comm.state =(bool) *(ptr+3);
		
		if(MacAsciiToInt(cp,mac)==1)
		{
			dev.type = 1;
			memcpy(dev.mac, mac, 6);
			dev.ip_addr.addr = ip_addr.addr;
			dev.time = GetLocalTime();	
			
			dev_insert_list(dev);
		}
	}
	
	
	udp_client_Send(&pserver->sudp, pserver->sudp.uip, p->payload, p->len);
	printf("[sw]: ");
	for(i=0;i<p->len;i++)
	{
		printf(" %X",rec[i] );
	}
	printf("\r\n");
#endif
  pbuf_free(p);
}


void Switch_find(void)
{
	switch_udp_Send(switch_infor.adv_ip, SwitchAdvCMD, sizeof(SwitchAdvCMD)-1);
}

//switch udp 发送
err_t switch_udp_Send(struct ip_addr addr, uint8_t *p, uint16_t len)
{
	err_t ret = ERR_OK;

	ret = udp_client_Send(&switch_infor.netlink.udp, addr, p, len);
	return ret;
}


static void  connectedf(void *arg)
{
//	tcp_struct_t  *ts;
	struct pbuf *p;
	
//	ts = (tcp_struct_t*)arg;
	p = pbuf_alloc(PBUF_TRANSPORT, PACKAGE_MAX, PBUF_RAM);
	
	memcpy(p->payload,connectedcmd,sizeof(connectedcmd));
	p->tot_len = p->len = sizeof(connectedcmd);
	udp_client_Send(&switch_infor.pdev->server->sudp, switch_infor.pdev->server->sudp.uip, p->payload, p->len);
	pbuf_free(p);
}	
static void connecterrf(void *arg)
{
//	tcp_struct_t  *ts;
	struct pbuf *p;
	
//	ts = (tcp_struct_t*)arg;
	p = pbuf_alloc(PBUF_TRANSPORT, PACKAGE_MAX, PBUF_RAM);
	
	memcpy(p->payload,connecterrcmd,sizeof(connecterrcmd));
	p->tot_len = p->len = sizeof(connecterrcmd);
	udp_client_Send(&switch_infor.pdev->server->sudp, switch_infor.pdev->server->sudp.uip, p->payload, p->len);
	pbuf_free(p);
}
static void connectclose(void *arg)
{
//	tcp_struct_t  *ts;
	struct pbuf *p;
	
//	ts = (tcp_struct_t*)arg;
	p = pbuf_alloc(PBUF_TRANSPORT, PACKAGE_MAX, PBUF_RAM);
	
	memcpy(p->payload,closecmd,sizeof(closecmd));
	p->tot_len = p->len = sizeof(closecmd);
	udp_client_Send(&switch_infor.pdev->server->sudp, switch_infor.pdev->server->sudp.uip, p->payload, p->len);
	pbuf_free(p);
}


//初始化TCP客户端
err_t Switch_TCP_Client_Attemp_Connect(smart_switch_infor_t  *ps)
{	
	return TCP_Client_Attemp_Connect(&ps->netlink.tcp);
}

//关闭TCP链接
void Switch_TCP_Client_Close( smart_switch_infor_t* ss)
{	
	tcp_client_close(&ss->netlink.tcp);
}

err_t Switch_TCP_Send(smart_switch_infor_t *es, uint8_t *msg, uint16_t len)
{
	if(es==NULL)
		return ERR_VAL;
	if(&es->netlink.tcp==NULL)
		return ERR_VAL;
	return TCP_Send(&es->netlink.tcp, msg, len);
}
err_t Switch_Tcp_Rec(struct pbuf *p, void *arg, err_t err)
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
		udp_client_Send(&switch_infor.pdev->server->sudp, switch_infor.pdev->server->sudp.uip, p->payload, p->len);
		pbuf_free(p); 
	}
	return err;
}



