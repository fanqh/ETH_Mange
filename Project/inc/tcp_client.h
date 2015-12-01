#ifndef __TCP_CLIENT_H_
#define __TCP_CLIENT_H_
#include "main.h"
#include "netif.h"
#include "device_server.h"
#include "netconf.h"
#include "net_type.h"


typedef struct
{
	tcp_struct_t *ptcp;
//	Dev_Server_infor_t *pserver;
	
}tcp_infor_t;

err_t TCP_Client_Attemp_Connect(tcp_infor_t *ts);
void tcp_client_close( tcp_infor_t* ts);
err_t TCP_Send(tcp_infor_t *es, uint8_t *msg, uint16_t len);


err_t TCP_Client_Attemp_Connect1(void);


#endif


