#ifndef __ETH_MANGE_H_
#define __ETH_MANGE_H_

#include "netif.h"
#include "stm32f10x.h"


typedef struct
{
	struct ip_addr ip_addr;
  struct ip_addr netmask;
  struct ip_addr gw;
	uint8_t macaddr[6];
	uint8_t is_connet;
	
}device_infor_t; 




#endif