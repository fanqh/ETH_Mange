#ifndef __DEVICE_SERVER_H_
#define __DEVICE_SERVER_H_

#include "netconf.h"


typedef struct
{
	uint8_t is_tcpser_connnect;
	struct udp_pcb  *upcb_server;
	struct tcp_pcb  *tcp_pcb_server;
	device_infor_t  *pDevInfor;
	
}Dev_Server_infor_t;


err_t udp_server_init(device_infor_t *pd);



#endif



