#include "main.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>



const uint8_t SmartSwich[] = "YZ-RECOSCAN";

/***********************************************************************
函数名称：My_IP4_ADDR(void)
功    能：IP地址的装配
***********************************************************************/
void SET_IP4_ADDR(struct ip_addr *ipaddr,unsigned char a,unsigned char b,unsigned char c,unsigned char d)
{
	ipaddr->addr = htonl(((u32_t)((a) & 0xff) << 24) | \
                               ((u32_t)((b) & 0xff) << 16) | \
                               ((u32_t)((c) & 0xff) << 8) | \
                                (u32_t)((d) & 0xff));
}


/**
  * @brief  Initialize the client application.
  * @param  None
  * @retval None
  */
		uint8_t Sent[]="fanqh test udp client\r\n";
void udp_broacast_(void)
{
   struct udp_pcb *upcb;
   struct pbuf *p;

   SET_IP4_ADDR(&ip_udp_server, UDP_SERVER_IP);                              
   /* Create a new UDP control block  */
   upcb = udp_new();   
   p = pbuf_alloc(PBUF_TRANSPORT, sizeof(Sent), PBUF_RAM);
	 p->payload = (void*)Sent; 
	 upcb->local_port = 5;
	 udp_connect(upcb, &ip_udp_server, UDP_SERVER_PORT);	 
	 udp_send(upcb, p); 
	   /* Reset the upcb */
   udp_disconnect(upcb);
   
   /* Bind the upcb to any IP address and the UDP_PORT port*/
   udp_bind(upcb, IP_ADDR_ANY, 5);
	
	
   udp_recv(upcb, udp_client_callback, NULL);

   /* Free the p buffer */
   pbuf_free(p);
  
}

