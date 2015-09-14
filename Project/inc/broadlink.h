#ifndef __BROADLINK_H_
#define __BROADLINK_H_

#include "netif.h"
#include "stm32f10x.h"

typedef struct{
	uint8_t is_connect;
	struct ip_addr ip_addr;
	
}broadlink_infor_t;



#endif


