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
//#define BROADLINK_IP_ADDR				255,255,255,255
#define BROADLINK_IP_ADDR				192,168,0,100

/* Private functions ---------------------------------------------------------*/
struct ip_addr broadlink_ip;
//struct udp_pcb  *pbroadlink_upcb;
static const uint8_t sm[] = {0xff,0xee,0x11, 0x00,0x00,0x11,0xef,0xfe};
broadlink_infor_t broadlink_infor;


static void broadlink_rec_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
extern void SET_IP4_ADDR(struct ip_addr *ipaddr,unsigned char a,unsigned char b,unsigned char c,unsigned char d);

void broadlink_broadcast_init(void)
{
   

   SET_IP4_ADDR(&broadlink_ip, BROADLINK_IP_ADDR);                              
   /* Create a new UDP control block  */
   broadlink_infor.upcb = udp_new();   
   p = pbuf_alloc(PBUF_TRANSPORT, 64, PBUF_RAM);
	 p->payload = (void*)sm; 
	 p->len =8;
	 p->tot_len = 8;
	 broadlink_infor.upcb->local_port = UDP_CLIENT_PORT;
	
	 udp_bind(broadlink_infor.upcb, IP_ADDR_ANY, UDP_CLIENT_PORT);
	 udp_connect(broadlink_infor.upcb, &broadlink_ip, BROADLINK_PORT);
	 udp_send(broadlink_infor.upcb, p); 
	 udp_disconnect(broadlink_infor.upcb);
   udp_recv(broadlink_infor.upcb, broadlink_rec_callback, NULL);
	   /* Reset the pbroadlink_upcb */
   /* Free the p buffer */
   pbuf_free(p);
}


err_t Broadlink_Send(uint8_t *p, uint16 len)
{
	
}

err_t Broadlink_Broadcast(uint8_t *passwd)
{		
		err_t ret = ERR_OK;
		struct pbuf *p;
		uint8_t BrocastBuff[8] = {0xff,0xee,0x11, 0x00,0x00,0x11,0xef,0xfe};
																
		udp_pcb *upcb = udp_new(); 
		if(upcb==NULL)
			return ERR_BUF;
		
		p = pbuf_alloc(PBUF_TRANSPORT, 64, PBUF_RAM);
		if(p==NULL)
			return ERR_BUF;
		
		BrocastBuff[5] = 0x11+*passwd+*(passwd+1);
		SET_IP4_ADDR(&broadlink_ip, BROADLINK_IP_ADDR); 
		
		p->payload = (void*)BrocastBuff; 
		p->len =8;
		p->tot_len = 8;
		broadlink_infor.upcb->local_port = UDP_CLIENT_PORT;

		ret = udp_bind(upcb, IP_ADDR_ANY, UDP_CLIENT_PORT);
		if(ret!=ERR_OK)
			return ret;
		udp_connect(upcb, &broadlink_ip, BROADLINK_PORT);
		ret = udp_send(upcb, p); 
		if(ret!= ERR_OK)
			return ret;
		udp_disconnect(upcb);
		udp_recv(broadlink_infor.upcb, broadlink_rec_callback, NULL);
		 /* Reset the pbroadlink_upcb */
		/* Free the p buffer */
		pbuf_free(p);	
		
		return ret;
}


void udp_diconnect(broadlink_infor_t *pDevice_Infor)
{
		
		pDevice_Infor->is_connect = 0;
}

void udp_transfer(broadlink_infor_t *pDevice_Infor, uint8_t *buff, uint16_t len)
{
	struct pbuf *p;
	
	p->payload = buff;
	p->len = p->tot_len = len;
	udp_send(broadlink_infor.upcb, p); 
}

static void broadlink_rec_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
  struct tcp_pcb *pcb;
	uint8_t rec[256];
	
	printf("the broadcast ip: %X\r\n", (uint32_t)addr->addr);
//	memcpy(rec, p->payload,p->len);
//	
//	if((rec[0]==0xFF) && (rec[1]==0xEE))
//	{
//		if(broadlink_infor.is_connect!=1)
//		{
//			broadlink_infor.is_connect = 1;
//			broadlink_infor.ip_addr.addr = addr->addr; 
//			broadlink_infor.upcb = upcb;
//		}
//	}
//	else 
//		return;
  pbuf_free(p);
}



