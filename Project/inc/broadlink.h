#ifndef __BROADLINK_H_
#define __BROADLINK_H_

#include "netif.h"
#include "stm32f10x.h"
#include "lwip/udp.h"


typedef struct{
	uint8_t is_connect;
	struct ip_addr ip_addr;
	struct udp_pcb  *upcb;
	
}broadlink_infor_t;



extern broadlink_infor_t broadlink_infor;



void broadlink_broadcast_init(void);

#endif


