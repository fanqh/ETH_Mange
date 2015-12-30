#ifndef _DEVICE_CONFIG_H_
#define _DEVICE_CONFIG_H_
#include "stm32f10x.h"
#include "lwip/udp.h"

#define TCPDEBUG 1
#define  SUPPORT_DEV_ONLINE_MAX  64




typedef struct 
{
	uint8_t type;  //1. reco 2.revogi 3.yaokongbao
	uint8_t mac[32];
	uint32_t time;
	struct ip_addr ip_addr;
}Device_t;

extern Device_t Dev_list[SUPPORT_DEV_ONLINE_MAX];
int dev_find_list(Device_t dev);
int dev_find_Null_list(void);
int dev_insert_list(Device_t dev);
int dev_refresh_list(Device_t dev);
int addr1Toaddr2len(char *addr1, char *addr2);

#endif


