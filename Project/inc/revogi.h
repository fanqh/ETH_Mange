#ifndef __REVOGI_H_
#define __REVOGI_H_

#include "netif.h"
#include "stm32f10x.h"
#include "lwip/udp.h"
#include "netconf.h"

struct byte
{
	uint8_t port1:1;
	uint8_t port2:1;
	uint8_t port3:1;
	uint8_t port4:1;
	uint8_t port5:1;
	uint8_t port6:1;
	uint8_t port7:1;
	uint8_t port8:1;
};

typedef union 
{
	uint8_t state;
	struct byte s;
}switch_state_t;


typedef struct
{
	char mac[12];
	char sn[16];
	char dver[4];//设备版本
	char sver[4];//服务器版本
	bool regist;
	bool protect;
	switch_state_t sw;
}revogi_comm_t;

typedef struct
{	
	uint8_t count;
	
	netlink_struct_t  net;
	struct ip_addr    adv_ip;
	revogi_comm_t     comm;

	device_infor_t  *pdev;
}revogi_infor_t;

extern revogi_infor_t revogi_infor;

err_t revogi_Init(device_infor_t *pDev);
err_t revogi_udp_Send(struct ip_addr addr, uint8_t *p, uint16_t len);
err_t PowerTrip_TCP_Client_Attemp_Connect(revogi_infor_t  *ps);
err_t PowerTrip_TCP_Send(revogi_infor_t *es, uint8_t *msg, uint16_t len);




#endif
