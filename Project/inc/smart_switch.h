#ifndef _SMART_SWITCH_H_
#define _SMART_SWITCH_H_

#include "netif.h"
#include "stm32f10x.h"
#include "lwip/udp.h"

typedef enum
{
	UNUSE = 0,
	DISCONNECT,
	CONNECTED	
}Sw_Infor_State_t;

typedef struct
{	
	Sw_Infor_State_t sw_state;
	uint8_t connect_count;
	struct ip_addr  udp_adv_ip;
	struct ip_addr  tcp_ip;
	struct udp_pcb  *udp_pcb;
	struct tcp_pcb  *tcp_pcb;
	struct pbuf *p;
	uint8_t mac[6];
	uint8_t sn[5];
	uint16_t udp_local_port;
	uint16_t udp_remote_port;
}smart_switch_infor_t;

err_t Switch_Init(void);
err_t switch_udp_Send(uint8_t *p, uint16_t len);

err_t Switch_TCP_Client_Attemp_Connect(smart_switch_infor_t  *ps);
//static err_t Switch_TCP_Client_Connected(void *arg, struct tcp_pcb *tpcb, err_t err);
uint8_t Switch_TCP_Send(smart_switch_infor_t *es, uint8_t *msg, uint16_t len);
uint8_t CompareMac(uint8_t *pmac1, uint8_t *pmac2);

#endif
