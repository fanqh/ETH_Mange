#ifndef __UDP_CLIENT_H_
#define __UDP_CLIENT_H_

#include "netif.h"
#include "stm32f10x.h"
#include "lwip/udp.h"
#include "netconf.h"
#include "net_type.h"

#define PACKAGE_MAX              256

#define  connectedcmd    "connnect:"
#define  connecterrcmd   "connecterr:"
#define  closecmd 		 "close:"

err_t udp_client_init(udp_struct_t *ut, void *arg);
err_t udp_client_Send(udp_struct_t *ut, struct ip_addr addr, uint8_t *p, uint16_t len);

uint8_t CompareMac(uint8_t *pmac1, uint8_t *pmac2);
int NumofStr(char*str, char c);


#endif
