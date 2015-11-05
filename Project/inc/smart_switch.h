#ifndef _SMART_SWITCH_H_
#define _SMART_SWITCH_H_

#include "netif.h"
#include "stm32f10x.h"
#include "lwip/udp.h"

typedef struct
{
	struct ip_addr adv_ip;
	struct ip_addr udp_ip;
	struct ip_addr tcp_ip;
	struct udp_pcb  *udp_pcb;
	struct udp_pcb  *tcp_pcb;
	uint8_t mac[6];
	uint8_t sn[5];
}smart_switch_infor_t;




#endif
