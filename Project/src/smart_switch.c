#include "main.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include <string.h>
#include <stdio.h>
#include "smart_switch.h"
#include "netconf.h"

#define SWTICH_ADV_PORT   		 48899
#define SWITCH_UPD_LOCAL_PORT  48899
smart_switch_infor_t switch_infor;

static void switch_rec_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
err_t broadlink_init(void)
{
	err_t ret = ERR_OK;
	
	switch_infor.adv_ip.addr = Device_Infor.gw.addr | 0xff000000;

	switch_infor.udp_pcb = udp_new();
	if(switch_infor.udp_pcb==NULL)
		return ERR_BUF;
	switch_infor.udp_pcb->local_port = SWTICH_ADV_PORT;
	ret = udp_bind(switch_infor.udp_pcb, IP_ADDR_ANY, SWITCH_UPD_LOCAL_PORT);
	if(ret!=ERR_OK)
	{
		udp_remove(switch_infor.udp_pcb);
		return ret;
	}
	ret = udp_connect(switch_infor.udp_pcb, &switch_infor.adv_ip, SWTICH_ADV_PORT);
	if(ret!=ERR_OK)
	{
		udp_remove(switch_infor.udp_pcb);
		return ret;
	}
	udp_recv(switch_infor.udp_pcb, switch_rec_callback, NULL);
	return ret;
}

static void switch_rec_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
//  struct tcp_pcb *pcb;
	uint8_t rec[256];
	uint8_t i;
	
	printf("[switch]: broadcast ip: %X\r\n", (uint32_t)addr->addr);
	memcpy(rec, p->payload,p->len);
	printf("[switch]: ");
	for(i=0;i<p->len;i++)
	{
		printf(" %X",rec[i] );
	}
	printf("\r\n");

  pbuf_free(p);
}


err_t switch_udp_Send(uint8_t *p, uint16_t len)
{
		struct pbuf *pSend;
		err_t ret = ERR_OK;
	
		pSend = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
		if(pSend==NULL)
			return ERR_BUF;
		pSend->payload = p;
		pSend->tot_len = pSend->len = len;
		
		ret = udp_connect(switch_infor.udp_pcb, &switch_infor.adv_ip, SWTICH_ADV_PORT);
		if(ret!=ERR_OK)
		{
			udp_remove(switch_infor.udp_pcb);
			return ret;
		}
		ret = udp_send(switch_infor.udp_pcb, pSend); 
		if(ret!=ERR_OK)
			printf("send failed\r\n");
		udp_disconnect(switch_infor.udp_pcb);
		
		pbuf_free(pSend);
		return ret;
}




