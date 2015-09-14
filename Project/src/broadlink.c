/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include <string.h>
#include <stdio.h>
#include "broadlink.h"


/* Private typedef -----------------------------------------------------------*/
#define BROADLINK_PORT      		9001
#define UDP_CLIENT_PORT      		9002
#define BROADLINK_IP_ADDR				255,255,255,255


/* Private functions ---------------------------------------------------------*/
struct ip_addr broadlink_ip;
struct udp_pcb  *pbroadlink_upcb;
static const uint8_t sm[] = {0xff,0xee,0x11, 0x00,0x00,0x11,0xef,0xfe};
broadlink_infor_t broadlink_infor;


static void broadlink_rec_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
extern void SET_IP4_ADDR(struct ip_addr *ipaddr,unsigned char a,unsigned char b,unsigned char c,unsigned char d);
void broadlink_broadcast_init(void)
{
   struct pbuf *p;

   SET_IP4_ADDR(&broadlink_ip, BROADLINK_IP_ADDR);                              
   /* Create a new UDP control block  */
   pbroadlink_upcb = udp_new();   
   p = pbuf_alloc(PBUF_TRANSPORT, sizeof(sm), PBUF_RAM);
	 p->payload = (void*)sm; 
	 p->len =sizeof(sm);
	 p->tot_len = sizeof(sm);
	 pbroadlink_upcb->local_port = UDP_CLIENT_PORT;
	 udp_connect(pbroadlink_upcb, &broadlink_ip, BROADLINK_PORT);	 
	 udp_send(pbroadlink_upcb, p); 
	   /* Reset the pbroadlink_upcb */
 //  udp_disconnect(pbroadlink_upcb);
   
   /* Bind the pbroadlink_upcb to any IP address and the UDP_PORT port*/
   udp_bind(pbroadlink_upcb, IP_ADDR_ANY, UDP_CLIENT_PORT);
   udp_recv(pbroadlink_upcb, broadlink_rec_callback, NULL);

   /* Free the p buffer */
   pbuf_free(p);
}

static void broadlink_rec_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
  struct tcp_pcb *pcb;
	uint8_t rec[256];
	
	printf("the broadcast ip: %X", (uint32_t)addr->addr);
	memcpy(rec, p->payload,p->len);
	
	if((rec[0]==0xFF) && (rec[1]==0xEE))
	{
		if(broadlink_infor.is_connect!=1)
		{
			broadlink_infor.is_connect = 1;
			broadlink_infor.ip_addr.addr = addr->addr; 
		}
	}
	else 
		return;
  pbuf_free(p);
}



