#ifndef _SMART_SWITCH_H_
#define _SMART_SWITCH_H_

#include "netif.h"
#include "stm32f10x.h"
#include "lwip/udp.h"
#include "netconf.h"
#include "device_server.h"
#include "net_type.h"


/*
typedef struct
{
	struct ip_addr  udp_adv_ip;
	struct udp_pcb  *upcb;
	uint16_t uremote_port;
	uint16_t ulocal_port;
}udp_struct_t;
typedef struct
{
	uint16_t retry;
	tcp_state_t tstate;
	struct ip_addr  tip;
	struct tcp_pcb  *tpcb;
	uint16_t tlocal_port;
	uint16_t tremote_port;
	err_t (* recv)(struct tcp_pcb *tpcb,struct pbuf *p,void *arg, err_t err);
	uint8_t mac[12];	
}tcp_struct_t;

*/



typedef struct
{	
	uint8_t connect_count;

	udp_struct_t udp;
	tcp_struct_t tcp;
	
	struct ip_addr  adv_ip;
	uint8_t sn[9];
	bool is_online;
	bool state;

	device_infor_t  *pdev;
}smart_switch_infor_t;


extern smart_switch_infor_t switch_infor;

err_t Switch_Init(device_infor_t *pDev);
err_t switch_udp_Send(struct ip_addr addr, uint8_t *p, uint16_t len);

err_t Switch_TCP_Client_Attemp_Connect(smart_switch_infor_t  *ps);
//static err_t Switch_TCP_Client_Connected(void *arg, struct tcp_pcb *tpcb, err_t err);
err_t Switch_TCP_Send(smart_switch_infor_t *es, uint8_t *msg, uint16_t len);
err_t Switch_Tcp_Rec(struct tcp_pcb *tpcb,struct pbuf *p, void *arg, err_t err);
uint8_t CompareMac(uint8_t *pmac1, uint8_t *pmac2);
int NumofStr(char*str, char c);
//const uint8_t SwitchAdvCMD[];
#endif
