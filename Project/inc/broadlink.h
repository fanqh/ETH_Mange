#ifndef __BROADLINK_H_
#define __BROADLINK_H_

#include "netif.h"
#include "stm32f10x.h"
#include "lwip/udp.h"
#include "netconf.h"

#define UDP_PACKAGE_MAX  			256

#define 		FIND_CMD 						0x11
#define   	KEEPALIVE_CMD   		0x22
#define   	QUERY_CMD      			0XA3


/*
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
*/

typedef enum
{
		BL_UNINIT,
		BL_INITIALIZED,
		BL_CONNECTED,
}device_state_t;

typedef union
{
	uint16_t code;
	uint8_t  c[2];
}u16u8_def_t;	


typedef struct
{
	u16u8_def_t pswd;
}Broadlink_comm_t;


typedef struct{
	uint8_t count;
	netlink_struct_t net;
	Broadlink_comm_t  comm;;
	struct ip_addr    adv_ip;
	device_infor_t  *pdev;
	
	device_infor_t  *pdev;
	
}broadlink_infor_t;



extern broadlink_infor_t broadlink_infor;


err_t Broadlink_Find(uint8_t *passwd);
err_t broadlink_init(device_infor_t *pDec);
err_t Broadlink_KeepAlive(void);
err_t Broadlink_Query(uint8_t check_state, uint16_t port, uint8_t *id );

#endif


