#ifndef __UDP_CLIENT_H_
#define __UDP_CLIENT_H_

#include "netif.h"
#include "stm32f10x.h"
#include "lwip/udp.h"


err_t udp_client_init(struct udp_pcb  *upcb, void (* recv)(void *arg, struct udp_pcb *upcb, struct pbuf *p,
                     struct ip_addr *addr, u16_t port),struct ip_addr ip_addr, uint16_t remote_port, uint16_t local_port);
err_t udp_client_Send(struct udp_pcb  *upcb, struct ip_addr ip_addr, uint16_t port, uint8_t *p, uint16_t len);




#endif
