#ifndef __BROADLINK_H_
#define __BROADLINK_H_

#include "netif.h"
#include "stm32f10x.h"
#include "lwip/udp.h"

#define UDP_PACKAGE_MAX  			256

#define 	FIND_CMD 				0x11
#define   	KEEPALIVE_CMD   		0x22
#define   	QUERY_CMD      			0XA3


typedef struct{
	uint8_t is_connect;
	struct ip_addr ip_addr;
	struct udp_pcb  *upcb;
	
}broadlink_infor_t;



extern broadlink_infor_t broadlink_infor;


err_t Broadlink_Find(uint8_t *passwd);

#endif


