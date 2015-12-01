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

#define SWTICH_ADV_PORT   			 48899
#define SWITCH_UPD_LOCAL_PORT  	 48899

#define SWITCH_TCP_PORT          8899
#define SWITCH_TCP_LOCAL_PORT    9008

#define PACKAGE_MAX              256


smart_switch_infor_t switch_infor=
{
	0,
	{S_IDLE, 0,0,SWTICH_ADV_PORT,SWITCH_UPD_LOCAL_PORT,0},//udp
	{0,S_IDLE,0,0,SWITCH_TCP_LOCAL_PORT,SWITCH_TCP_PORT,Switch_Tcp_Rec,{0}},//tcp
	{0}, //adv_ip
	{0},
	FALSE,
	FALSE,
	0
};
struct ip_addr adv_ip;
uint8_t udp_init =0;
struct tcp_pcb *TcpPCB;

//const uint8_t SwitchAdvCMD[] = "YZ-RECOSCAN";

static void switch_rec_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);


//switch 初始化
err_t Switch_Init(device_infor_t *pDev)
{
	
	err_t ret = ERR_OK;
	
	switch_infor.adv_ip.addr = Device_Infor.pnetif->gw.addr | 0xff000000;
	switch_infor.udp.upcb = udp_new();
	if(switch_infor.udp.upcb==NULL)
		return ERR_BUF;
	pDev->udp_num++;
	switch_infor.pdev = pDev;
	switch_infor.udp.recv = switch_rec_callback;
	switch_infor.tcp.arg = pDev->server;
//	switch_infor.udp.uremote_port = SWTICH_ADV_PORT;
//	switch_infor.udp.ulocal_port = SWITCH_UPD_LOCAL_PORT;
//	switch_infor.tcp_ip.addr = 0;
	
	ret = udp_client_init(&switch_infor.udp, &switch_infor);
	return ret;
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
	
	pSw = (smart_switch_infor_t*)arg;
	pserver = pSw->pdev->server;
	
	printf("[sw]: broadcast ip: %X\r\n", (uint32_t)addr->addr);
	memset(rec,0,PACKAGE_MAX);
	memcpy(rec, p->payload, p->len);
	if((NumofStr(rec, '.')==3)&&(NumofStr(rec, ',')==5))
	{
		if(inet_aton(rec, (struct in_addr*)&ip_addr)==0)
			return;
		if(ip_addr.addr!=addr->addr)
			return;
		switch_infor.tcp.tip.addr = ip_addr.addr;
		switch_infor.udp.uip.addr = ip_addr.addr;
		
		ptr = strstr((char*)rec, ",");
		if(ptr==NULL)
			return;
		memcpy(switch_infor.tcp.mac, ptr+1,12);
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
	
	
	udp_client_Send(&pserver->sudp, pserver->sudp.uip, p->payload, p->len);
	printf("[sw]: ");
	for(i=0;i<p->len;i++)
	{
		printf(" %X",rec[i] );
	}
	printf("\r\n");

  pbuf_free(p);
}

//switch udp 发送
err_t switch_udp_Send(struct ip_addr addr, uint8_t *p, uint16_t len)
{
		err_t ret = ERR_OK;
	
		ret = udp_client_Send(&switch_infor.udp, addr, p, len);
		return ret;
}



//初始化TCP客户端
err_t Switch_TCP_Client_Attemp_Connect(smart_switch_infor_t  *ps)
{	
	tcp_struct_t *ts;
//	ts->pserver = ps->pdev->server;
//	ts->ptcp = &(ps->tcp);
	return  TCP_Client_Attemp_Connect1();
}

//关闭TCP链接
void Switch_TCP_Client_Close( smart_switch_infor_t* ss)
{
	tcp_infor_t *ts;
	
	ts->ptcp = &(ss->tcp);
	
	tcp_client_close(ts);
}

err_t Switch_TCP_Send(smart_switch_infor_t *es, uint8_t *msg, uint16_t len)
{
	tcp_infor_t *ts;

	if(es==NULL)
		return ERR_VAL;
	if((ts->ptcp=&(es->tcp))==NULL)
		return ERR_VAL;
	return TCP_Send(ts, msg, len);
}
err_t Switch_Tcp_Rec(struct tcp_pcb *tpcb,struct pbuf *p, void *arg, err_t err)
{
	uint8_t rec[256];
	tcp_infor_t  *es;
	es = (tcp_infor_t*)arg;
	
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
