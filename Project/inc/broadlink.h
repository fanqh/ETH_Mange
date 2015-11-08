#ifndef __BROADLINK_H_
#define __BROADLINK_H_

#include "netif.h"
#include "stm32f10x.h"
#include "lwip/udp.h"

#define UDP_PACKAGE_MAX  			256

#define 		FIND_CMD 						0x11
#define   	KEEPALIVE_CMD   		0x22
#define   	QUERY_CMD      			0XA3

typedef enum
{
		BL_UNINIT,
		BL_INITIALIZED,
		BL_CONNECTED,
}device_state_t;



typedef struct{
	device_state_t state;
	uint16_t local_port;
	uint16_t remote_port;
	struct ip_addr ip_addr;
	struct udp_pcb  *upcb;
	
}broadlink_infor_t;



extern broadlink_infor_t broadlink_infor;


err_t Broadlink_Find(uint8_t *passwd);
err_t broadlink_init(void);
err_t Broadlink_KeepAlive(void);
err_t Broadlink_Query(uint8_t check_state, uint16_t port, uint8_t *id );

#endif


