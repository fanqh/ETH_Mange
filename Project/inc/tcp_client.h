#ifndef __TCP_CLIENT_H_
#define __TCP_CLIENT_H_
#include "main.h"
#include "netif.h"
#include "device_server.h"

typedef enum
{
	S_IDLE,
	S_CONNECTING,
	S_CONNECTED,
	S_CLOSING,
	S_CLOSED,
	S_SEND,
	S_RECEIVE,
	
}tcp_state_t;


typedef struct
{
	tcp_state_t state;
	struct tcp_pcb *tpcb;	
	struct ip_addr  ip;
	uint16_t local_port;
	uint16_t remote_port;
	uint16_t retry;
//	err_t (* connected)(void *arg, struct tcp_pcb *tpcb, err_t err);
//	void *arg;
//	void (* errf)(void *arg, err_t err);
	err_t (* recv)(struct tcp_pcb *tpcb,struct pbuf *p,void *arg);
	Dev_Server_infor_t *pserver;
	
}tcp_infor_t;

err_t TCP_Client_Attemp_Connect(tcp_infor_t *ts);
void tcp_client_close( tcp_infor_t* ts);
err_t TCP_Send(tcp_infor_t *es, uint8_t *msg, uint16_t len);


#endif


