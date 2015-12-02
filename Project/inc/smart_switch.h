#ifndef _SMART_SWITCH_H_
#define _SMART_SWITCH_H_

#include "netif.h"
#include "stm32f10x.h"
#include "lwip/udp.h"
#include "netconf.h"
#include "device_server.h"
#include "net_type.h"

typedef struct
{
	uint8_t mac[12];
	uint8_t sn[9];
	bool is_online;
	bool state;
}smartplug_comm_t;


typedef struct
{	
	uint8_t connect_count;

	netlink_struct_t  netlink;
	smartplug_comm_t  smartplug_comm;
	struct ip_addr    adv_ip;
	
	device_infor_t  *pdev;
}smart_switch_infor_t;


extern smart_switch_infor_t switch_infor;

err_t Switch_Init(device_infor_t *pDev);
err_t switch_udp_Send(struct ip_addr addr, uint8_t *p, uint16_t len);

err_t Switch_TCP_Client_Attemp_Connect(smart_switch_infor_t  *ps);
//static err_t Switch_TCP_Client_Connected(void *arg, struct tcp_pcb *tpcb, err_t err);
err_t Switch_TCP_Send(smart_switch_infor_t *es, uint8_t *msg, uint16_t len);
//err_t Switch_Tcp_Rec(struct tcp_pcb *tpcb,struct pbuf *p, void *arg, err_t err);

//const uint8_t SwitchAdvCMD[];
#endif
