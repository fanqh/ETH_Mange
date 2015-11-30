#ifndef _SMART_SWITCH_H_
#define _SMART_SWITCH_H_

#include "netif.h"
#include "stm32f10x.h"
#include "lwip/udp.h"
#include "netconf.h"
#include "device_server.h"


/*
typedef struct
{
	tcp_state_t state;

	struct tcp_pcb *tpcb;	
	struct ip_addr  ip;
	uint16_t local_port;
	uint16_t remote_port;
	uint16_t retry;
	err_t (* recv)(struct tcp_pcb *tpcb,struct pbuf *p,void *arg);
	Dev_Server_infor_t *pserver;
	
}tcp_infor_t;
*/

typedef struct
{	
//	Sw_Infor_State_t sw_state;
	uint8_t connect_count;
	
	
	struct ip_addr  udp_adv_ip;
	struct udp_pcb  *udp_pcb;
	uint16_t uremote_port;
	uint16_t ulocal_port;
	
	tcp_struct_t tcp;
//	uint16_t retry;
//	tcp_state_t tstate;
//	struct ip_addr  tip;
//	struct tcp_pcb  *tpcb;
//	uint16_t tlocal_port;
//	uint16_t tremote_port;
//	err_t (* recv)(struct tcp_pcb *tpcb,struct pbuf *p,void *arg);
//	uint8_t mac[12];
//	struct pbuf *p;
	
	uint8_t sn[9];
	bool is_online;
	bool state;

	device_infor_t  *pdev;
}smart_switch_infor_t;


extern smart_switch_infor_t switch_infor;

err_t Switch_Init(device_infor_t *pDev);
err_t switch_udp_Send(uint8_t *p, uint16_t len);

err_t Switch_TCP_Client_Attemp_Connect(smart_switch_infor_t  *ps);
//static err_t Switch_TCP_Client_Connected(void *arg, struct tcp_pcb *tpcb, err_t err);
err_t Switch_TCP_Send(smart_switch_infor_t *es, uint8_t *msg, uint16_t len);
uint8_t CompareMac(uint8_t *pmac1, uint8_t *pmac2);
int NumofStr(char*str, char c);
//const uint8_t SwitchAdvCMD[];
#endif
