#ifndef __DEVICE_SERVER_H_
#define __DEVICE_SERVER_H_

#include "netconf.h"
typedef struct 
{
	struct udp_pcb  *upcb;
	struct ip_addr addr;
	uint16_t port;
}upcb_server_t;

typedef struct
{
	uint8_t is_tcpser_connnect;
	upcb_server_t   upcb_server;
	struct tcp_pcb  *tcp_pcb_server;
	device_infor_t  *pDevInfor;
	
}Dev_Server_infor_t;


err_t udp_server_init(device_infor_t *pd);
Dev_Server_infor_t* GetDev_server(void);



#endif


