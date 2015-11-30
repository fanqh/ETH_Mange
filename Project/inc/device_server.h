#ifndef __DEVICE_SERVER_H_
#define __DEVICE_SERVER_H_
#include "tcp.h"
#include "main.h"
#include "net_type.h"


typedef struct 
{
	struct udp_pcb  *upcb;
	struct ip_addr addr;
	uint16_t port;
}upcb_server_t;

//typedef struct
//{
//	s_state_t ustate;
//	struct ip_addr  udp_ip;
//	struct udp_pcb  *upcb;
//	uint16_t uremote_port;
//	uint16_t ulocal_port;
//	void (* recv)(void *arg, struct udp_pcb *upcb, struct pbuf *p,struct ip_addr *addr, u16_t port);
//}udp_struct_t;

typedef struct
{
	uint8_t is_tcpser_connnect;
//	upcb_server_t   upcb_server;
	udp_struct_t    sudp;
	tcp_struct_t    stcp;
	void *arg;
	
}Dev_Server_infor_t;


err_t udp_server_init(void *pd);
Dev_Server_infor_t* GetDev_server(void);



#endif



