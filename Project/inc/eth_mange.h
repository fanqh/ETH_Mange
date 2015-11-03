#ifndef __ETH_MANGE_H_
#define __ETH_MANGE_H_

#include "netif.h"
#include "stm32f10x.h"



typedef struct
{
		uint32_t ip_addr;
		uint8_t mac[6];
		uint8_t SN;
		uint8_t flag1;
		uint8_t flag2;
		uint8_t sw_state[8]; //8·����״̬
		uint8_t is_connet;
		uint8_t retry_connect_count;
	
	
}smart_sw_infor_t;



#endif
