#ifndef __REVOGI_H_
#define __REVOGI_H_

#include "netif.h"
#include "stm32f10x.h"
#include "lwip/udp.h"
#include "netconf.h"


typedef struct
{	
	struct ip_addr  udp_adv_ip;
	struct ip_addr  tcp_ip;
	struct udp_pcb  *udp_pcb;
	struct tcp_pcb  *tcp_pcb;
	struct pbuf *p;
	uint8_t mac[12];
	uint8_t sn[9];
	bool is_online;
	bool state;
	uint16_t udp_local_port;
	uint16_t udp_remote_port;
}revogi_infor_t;






#endif
