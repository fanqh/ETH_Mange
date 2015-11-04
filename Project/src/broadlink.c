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
broadlink_infor_t broadlink_infor;
//struct udp_pcb  *pbroadlink_upcb;

uint8_t BroadlinktFind[8] = {0xff,0xee,FIND_CMD, 0x00,0x00,0x11,0xef,0xfe};
uint8_t BroadlinkKeepAlive[6] = {0xff,0xee,KEEPALIVE_CMD, 0x22,0xef,0xfe};
uint8_t BroadlinkQuery[] = {0xff,0xee,QUERY_CMD, 0x22,0xef,0xfe};


static void broadlink_rec_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
extern void SET_IP4_ADDR(struct ip_addr *ipaddr,unsigned char a,unsigned char b,unsigned char c,unsigned char d);


#if 1
err_t broadlink_init(void)
{
	err_t ret = ERR_OK;
	SET_IP4_ADDR(&broadlink_infor.ip_addr, BROADLINK_IP_ADDR);
	broadlink_infor.upcb = udp_new();
	if(broadlink_infor.upcb==NULL)
		return ERR_BUF;
	broadlink_infor.upcb->local_port = UDP_CLIENT_PORT;
	ret = udp_bind(broadlink_infor.upcb, IP_ADDR_ANY, UDP_CLIENT_PORT);
	if(ret!=ERR_OK)
	{
		udp_remove(broadlink_infor.upcb);
		return ret;
	}
	ret = udp_connect(broadlink_infor.upcb, &broadlink_infor.ip_addr, BROADLINK_PORT);
	if(ret!=ERR_OK)
	{
		udp_remove(broadlink_infor.upcb);
		return ret;
	}
	udp_recv(broadlink_infor.upcb, broadlink_rec_callback, NULL);
	return ret;
}

err_t Broadlink_Send(uint8_t *p, uint16_t len)
{
		struct pbuf *pSend;
		err_t ret = ERR_OK;
	
		pSend = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
		if(pSend==NULL)
			return ERR_BUF;
		pSend->payload = p;
		pSend->tot_len = pSend->len = len;
		
		ret = udp_connect(broadlink_infor.upcb, &broadlink_infor.ip_addr, BROADLINK_PORT);
		if(ret!=ERR_OK)
		{
			udp_remove(broadlink_infor.upcb);
			return ret;
		}
		ret = udp_send(broadlink_infor.upcb, pSend); 
		udp_disconnect(broadlink_infor.upcb);
		if(ret!=ERR_OK)
			printf("send failed\r\n");
		pbuf_free(pSend);
		return ret;
}
#else
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
#endif


err_t Broadlink_Send_Rece(uint8_t *p, uint16_t len)
{
			err_t ret = ERR_OK;
			struct pbuf *pSend;
			uint8_t temp[30];
	
			
			memcpy(temp, p,len);
			pSend = pbuf_alloc(PBUF_TRANSPORT, 256, PBUF_RAM);
			if(pSend==NULL)
				return ERR_BUF;
			
			SET_IP4_ADDR(&broadlink_ip, BROADLINK_IP_ADDR); 
			pSend->payload = p; 
			pSend->len = pSend->tot_len = len;
			broadlink_infor.upcb->local_port = UDP_CLIENT_PORT;

//			udp_connect(broadlink_infor.upcb, &broadlink_ip, BROADLINK_PORT);
			ret = udp_send(broadlink_infor.upcb, pSend); 
			if(ret!= ERR_OK)
			{
				pbuf_free(pSend);
				return ret;
			}
			udp_disconnect(broadlink_infor.upcb);
//			
//			ret = udp_bind(broadlink_infor.upcb, IP_ADDR_ANY, UDP_CLIENT_PORT);
//			if(ret!=ERR_OK)
//			{
//				udp_remove(broadlink_infor.upcb);
//				pbuf_free(pSend);
//				return ret;
//			}
//			udp_recv(broadlink_infor.upcb, broadlink_rec_callback, NULL);
			pbuf_free(pSend);	
		
			printf("[BDLINK]: send broadcast\r\n");
			return ret;
}

err_t Broadlink_Find(uint8_t *passwd)
{		
		
															
		BroadlinktFind[5] =  FIND_CMD + *passwd + *(passwd+1);		
		return Broadlink_Send(BroadlinktFind, sizeof(BroadlinktFind));
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
	
	printf("[BDLINK]: the broadcast ip: %X\r\n", (uint32_t)addr->addr);
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



