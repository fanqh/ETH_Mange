/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include <string.h>
#include <stdio.h>


err_t udp_client_init(struct udp_pcb  *upcb, void (* recv)(void *arg, struct udp_pcb *upcb, struct pbuf *p,
                     struct ip_addr *addr, u16_t port),struct ip_addr ip_addr, uint16_t remote_port, uint16_t local_port, void *arg)
{
	err_t ret = ERR_OK;
//	upcb->local_port = UDP_CLIENT_PORT;
	ret = udp_bind(upcb, IP_ADDR_ANY, local_port);
	if(ret!=ERR_OK)
	{
		udp_remove(upcb);
		return ret;
	}
	udp_recv(upcb, recv, arg);
	return ret;
}

err_t udp_client_Send(struct udp_pcb  *upcb, struct ip_addr ip_addr, uint16_t port, uint8_t *p, uint16_t len)
{
		struct pbuf *pSend;
		err_t ret = ERR_OK;
	
		pSend = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
		if(pSend==NULL)
			return ERR_BUF;
		pSend->payload = p;
		pSend->tot_len = pSend->len = len;
		
		ret = udp_connect(upcb, &ip_addr, port);
		if(ret!=ERR_OK)
		{
			udp_remove(upcb);
			return ret;
		}
		ret = udp_send(upcb, pSend); 
		if(ret!=ERR_OK)
			printf("send failed\r\n");
		udp_disconnect(upcb);
		
		pbuf_free(pSend);
		return ret;
}

