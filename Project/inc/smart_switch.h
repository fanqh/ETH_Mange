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
}Sw_Infor_State_t

typedef struct
{	
	Sw_Infor_State_t sw_state;
	uint8_t connect_count;
	struct ip_addr  tcp_ip;
	struct udp_pcb  *udp_pcb;
	struct tcp_pcb  *tcp_pcb;
	uint8_t mac[6];
	uint8_t sn[5];
}smart_switch_infor_t;


uint8_t CompareMac(uint8_t *pmac1, uint8_t *pmac2);

#endif
