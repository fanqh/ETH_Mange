#ifndef __NETCONF_H_
#define __NETCONF_H_
#include "stm32f10x.h"
#include "netif.h"


typedef enum
{
	S_IDLE,
	S_CONNECTING,
	S_CONNECTED,
	S_CLOSING,
	S_CLOSED,
	S_SEND,
	S_RECEIVE,
	
}s_state_t;	

typedef struct
{
	uint16_t retry;
	s_state_t tstate;
	struct ip_addr  tip;
	struct tcp_pcb  *tpcb;
	uint16_t tlocal_port;
	uint16_t tremote_port;
	err_t (* recv)(struct pbuf *p,void *arg, err_t err);
	void (* connectedf)(void *arg);	
	void (*connecterrf)(void *arg);
	void (*connectclose)(void *arg);
	
	void *arg;
}tcp_struct_t;

typedef struct
{
	s_state_t ustate;
	struct ip_addr  uip;
	struct udp_pcb  *upcb;
	uint16_t uremote_port;
	uint16_t ulocal_port;
	void (* recv)(void *arg, struct udp_pcb *upcb, struct pbuf *p,struct ip_addr *addr, u16_t port);
}udp_struct_t;

typedef struct
{
	udp_struct_t udp;
	tcp_struct_t tcp;
	
}netlink_struct_t;

#endif




