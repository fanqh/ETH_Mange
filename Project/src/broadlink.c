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
//#define BROADLINK_IP_ADDR				192,168,0,100

/* Private functions ---------------------------------------------------------*/
struct ip_addr broadlink_ip;
//struct udp_pcb  *pbroadlink_upcb;
static const uint8_t sm[] = {0xff,0xee,0x11, 0x00,0x00,0x11,0xef,0xfe};
broadlink_infor_t broadlink_infor;
uint8_t BroadlinktFind[8] = {0xff,0xee,FIND_CMD, 0x00,0x00,0x11,0xef,0xfe};
uint8_t BroadlinkKeepAlive[6] = {0xff,0xee,KEEPALIVE_CMD, 0x22,0xef,0xfe};
uint8_t BroadlinkQuery[] = {0xff,0xee,QUERY_CMD, 0x22,0xef,0xfe};


static void broadlink_rec_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
extern void SET_IP4_ADDR(struct ip_addr *ipaddr,unsigned char a,unsigned char b,unsigned char c,unsigned char d);

void broadlink_broadcast_init(void)
{
   
	struct pbuf *p;
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


err_t Broadlink_Send_Rece(uint8_t *p, uint16_t len)
{
			err_t ret = ERR_OK;
			struct pbuf *pSend;
			uint8_t temp[30];
	
			
			struct udp_pcb *upcb = udp_new(); 
			if(upcb==NULL)
				return ERR_BUF;
			memcpy(temp, p,len);
			pSend = pbuf_alloc(PBUF_TRANSPORT, 256, PBUF_RAM);
			if(pSend==NULL)
				return ERR_BUF;
			
			SET_IP4_ADDR(&broadlink_ip, BROADLINK_IP_ADDR); 
			pSend->payload = p; 
			pSend->len = pSend->tot_len = len;
			upcb->local_port = UDP_CLIENT_PORT;

			udp_connect(upcb, &broadlink_ip, BROADLINK_PORT);
			ret = udp_send(upcb, pSend); 
			if(ret!= ERR_OK)
			{
				pbuf_free(pSend);
				return ret;
			}
			udp_disconnect(upcb);
			
			ret = udp_bind(upcb, IP_ADDR_ANY, UDP_CLIENT_PORT);
			if(ret!=ERR_OK)
			{
				udp_remove(upcb);
				pbuf_free(pSend);
				return ret;
			}
			udp_recv(upcb, broadlink_rec_callback, NULL);
			pbuf_free(pSend);	
		
			printf("[BDLINK]: send broadcast\r\n");
			return ret;
}

err_t Broadlink_Find(uint8_t *passwd)
{		
		
															
		BroadlinktFind[5] =  FIND_CMD + *passwd + *(passwd+1);		
		return Broadlink_Send_Rece(BroadlinktFind, sizeof(BroadlinktFind));
}

err_t Broadlink_KeepAlive(void)
{
		return Broadlink_Send_Rece(BroadlinkKeepAlive, sizeof(BroadlinkKeepAlive));
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
//  struct tcp_pcb *pcb;
	uint8_t rec[256];
	uint8_t i;
	
	printf("the broadcast ip: %X\r\n", (uint32_t)addr->addr);
	memcpy(rec, p->payload,p->len);
	printf("[BDLINK]: ");
	for(i=0;i<p->len;i++)
	{
		printf(" %X",rec[i] );
	}
	printf("\r\n");
//}
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



