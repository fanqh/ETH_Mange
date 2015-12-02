/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include <string.h>
#include <stdio.h>
#include "broadlink.h"
#include "udp_client.h"
#include "device_server.h"

/* Private typedef -----------------------------------------------------------*/
#define BROADLINK_UDP_REMOTE_PORT      		9001
#define BROADLINK_UDP_LOCAL_PORT      		9002
#define BROADLINK_IP_ADDR				255,255,255,255
//#define BROADLINK_IP_ADDR				192,168,0,100



uint8_t SendBuff[UDP_PACKAGE_MAX];


uint8_t BroadlinktFind[8] = {0xff,0xee,FIND_CMD, 0x00,0x00,0x11,0xef,0xfe};
uint8_t BroadlinkKeepAlive[6] = {0xff,0xee,KEEPALIVE_CMD, 0x22,0xef,0xfe};
uint8_t BroadlinkQuery[] = {0xff,0xee,QUERY_CMD, 0,0xff,0,0,0x05,0,0,0,0,0,0, 0xef,0xfe};
uint8_t BroadlinkFBI[] = {0xFF,0XEE,0XD5,0,0XFF,0,0,0,0,0X0B,0,0,0,0,0,0,0,0,0,0,0,0XEF,0XFE};
//uint8_t BroadlinkRFSend[[] = {0xFF,0xEE,0xD3,0,0,0,0,0,};   //射频变长
//uint8_t BroadlinkInfrared[[] = //红外 变长
uint8_t BroadlinkStudy[]={0xFF,0xEE,0xB1,0,0,0,0,5,0,0,0,0,0,0xEF,0xFE};	
uint8_t BroadlinkCancelStudy[] = {0xFF,0xEE,0xc1,0,0,0,0,5,0,0,0,0,0,0,0xef,0xfe};


static void broadlink_rec_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
extern void SET_IP4_ADDR(struct ip_addr *ipaddr,unsigned char a,unsigned char b,unsigned char c,unsigned char d);

/* Private functions ---------------------------------------------------------*/
broadlink_infor_t broadlink_infor = 
{
	0,
	{{S_IDLE,0,0,BROADLINK_UDP_REMOTE_PORT,BROADLINK_UDP_LOCAL_PORT,broadlink_rec_callback},{0}},
	0
};

err_t broadlink_init(device_infor_t *pDec)
{
	err_t ret = ERR_OK;
	
	broadlink_infor.upcb = udp_new();
	if(broadlink_infor.upcb==NULL)
		return ERR_BUF;
	
	broadlink_infor.pdev = pDec;
	pDec->udp_num++;
	broadlink_infor.state = BL_UNINIT;
	broadlink_infor.local_port = UDP_CLIENT_PORT;
	broadlink_infor.remote_port = BROADLINK_PORT;
	SET_IP4_ADDR(&broadlink_infor.ip_addr, BROADLINK_IP_ADDR);
	
	ret = udp_client_init(broadlink_infor.upcb, broadlink_rec_callback, broadlink_infor.ip_addr, broadlink_infor.remote_port, broadlink_infor.local_port, &broadlink_infor);
	if(ret==ERR_OK)
		broadlink_infor.state = BL_INITIALIZED;
		
	return ret;
}

err_t Broadlink_Send(uint8_t *p, uint16_t len)
{
		err_t ret = ERR_OK;
	
		ret = udp_client_Send(broadlink_infor.upcb, broadlink_infor.ip_addr, broadlink_infor.remote_port, p, len);
		return ret;
}

err_t Broadlink_Find(uint8_t *passwd)
{		
		
															
		BroadlinktFind[5] =  FIND_CMD + *passwd + *(passwd+1);		
		return Broadlink_Send(BroadlinktFind, sizeof(BroadlinktFind));
}

err_t Broadlink_KeepAlive(void)
{
		return Broadlink_Send(BroadlinkKeepAlive, sizeof(BroadlinkKeepAlive));
}

err_t Broadlink_Query(uint8_t check_state, uint16_t port, uint8_t *id )
{
	uint8_t i;
	uint8_t crc = 0;
	uint8_t len;
	len = sizeof(BroadlinkQuery);
	
	BroadlinkQuery[3] = check_state;
	BroadlinkQuery[5] = (uint8_t) (port<<8)&0xff;
	BroadlinkQuery[12] = (uint8_t)(port&0xff);
	memcpy(&BroadlinkQuery[8],id,4);
	
	for(i=2; i<len-5; i++)
		crc += BroadlinkQuery[i];
	
	BroadlinkQuery[len-3] = crc;
	return Broadlink_Send(BroadlinkQuery, len);
}



//void udp_diconnect(broadlink_infor_t *pDevice_Infor)
//{
//		
//		pDevice_Infor->is_connect = 0;
//}



static void broadlink_rec_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
//  struct tcp_pcb *pcb;
	uint8_t rec[256];
	uint8_t i;
	broadlink_infor_t *pbroadlink;
	Dev_Server_infor_t *pserver;


	broadlink_infor.ip_addr.addr = addr->addr;
	pbroadlink = (broadlink_infor_t*)arg;
	pserver = (Dev_Server_infor_t*)pbroadlink->pdev->p;
	printf("[BDLINK]: broadcast ip: %X\r\n", (uint32_t)addr->addr);
	memcpy(rec, p->payload,p->len);
	printf("[BDLINK]: ");
	udp_client_Send(pserver->upcb_server.upcb, pserver->upcb_server.addr, pserver->upcb_server.port, p->payload, p->len);
	for(i=0;i<p->len;i++)
	{
		printf(" %X",rec[i] );
	}
	printf("\r\n");

   pbuf_free(p);
}



