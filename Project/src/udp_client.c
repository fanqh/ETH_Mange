/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include <string.h>
#include <stdio.h>
#include "device_server.h"
#include "lwip/opt.h"
#include "udp_client.h"



err_t udp_client_init(udp_struct_t *ut, void *arg)
{
	err_t ret = ERR_OK;
//	upcb->local_port = UDP_CLIENT_PORT;
	ret = udp_bind(ut->upcb, IP_ADDR_ANY, ut->ulocal_port);
	if(ret!=ERR_OK)
	{	
		ut->ustate = S_IDLE;
		udp_remove(ut->upcb);
		return ret;
	}
	ut->ustate = S_CONNECTED;
	udp_recv(ut->upcb, ut->recv, arg);
	return ret;
}

//err_t udp_client_Send(struct udp_pcb  *upcb, struct ip_addr ip_addr, uint16_t port, uint8_t *p, uint16_t len)
err_t udp_client_Send(udp_struct_t *ut, struct ip_addr addr, uint8_t *p, uint16_t len)
{
		struct pbuf *pSend;
		err_t ret = ERR_OK;
	
		if(ut->ustate==S_IDLE)
			return ERR_CLSD;
			
		pSend = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
		if(pSend==NULL)
			return ERR_BUF;
		memcpy(pSend->payload , p, len);
		pSend->tot_len = pSend->len = len;
		
		ret = udp_connect(ut->upcb, &addr, ut->uremote_port);
		if(ret!=ERR_OK)
		{
			udp_remove(ut->upcb);
			return ret;
		}
		ret = udp_send(ut->upcb, pSend); 
		if(ret!=ERR_OK)
			printf("send failed\r\n");
		udp_disconnect(ut->upcb);
		
		pbuf_free(pSend);
		return ret;
}

