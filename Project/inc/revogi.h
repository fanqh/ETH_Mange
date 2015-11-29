#ifndef __REVOGI_H_
#define __REVOGI_H_

#include "netif.h"
#include "stm32f10x.h"
#include "lwip/udp.h"
#include "netconf.h"


typedef struct
{	
	struct ip_addr  udp_adv_ip;
	struct ip_addr  tcp_ip;
	struct udp_pcb  *udp_pcb;
	struct tcp_pcb  *tcp_pcb;
	struct pbuf *p;
	uint8_t mac[12];
	uint8_t sn[9];
	bool is_online;
	bool state;
	uint16_t udp_local_port;
	uint16_t udp_remote_port;
	device_infor_t  *pdev;
}revogi_infor_t;

extern revogi_infor_t revogi_infor;

err_t revogi_Init(device_infor_t *pDev);
err_t revogi_find_udp_Send(uint8_t *p, uint16_t len);
err_t PowerTrip_TCP_Client_Attemp_Connect(revogi_infor_t  *ps);
err_t PowerTrip_TCP_Send(revogi_infor_t *es, uint8_t *msg, uint16_t len);




#endif
