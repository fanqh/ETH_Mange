#ifndef __UDP_CLIENT_H_
#define __UDP_CLIENT_H_

#include "netif.h"
#include "stm32f10x.h"
#include "lwip/udp.h"
#include "netconf.h"
#include "net_type.h"


err_t udp_client_init(udp_struct_t *ut, void *arg);
err_t udp_client_Send(udp_struct_t *ut, struct ip_addr *addr, uint8_t *p, uint16_t len);




#endif
