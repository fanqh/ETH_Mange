#include "revogi.h"
#include "udp_client.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include <string.h>
#include <stdio.h>
//#include "ip_addr.h"

#define  REVOGI_ADV_REMOTE_PORT  8888
#define  REVOGI_ADV_LOCAL_PORT   8889
#define  BUFF_LEN_MAX            256



revogi_infor_t revogi_infor;

static void revogi_rec_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);


//switch 初始化
err_t revogi_Init(device_infor_t *pDev)
{
	
	err_t ret = ERR_OK;
	
	revogi_infor.udp_adv_ip.addr = Device_Infor.pnetif->gw.addr | 0xff000000;
	revogi_infor.udp_pcb = udp_new();
	if(revogi_infor.udp_pcb==NULL)
		return ERR_BUF;
	pDev->udp_num++;
	revogi_infor.udp_remote_port = REVOGI_ADV_REMOTE_PORT;
	revogi_infor.udp_local_port = REVOGI_ADV_LOCAL_PORT;
	
	ret = udp_client_init(revogi_infor.udp_pcb, revogi_rec_callback, revogi_infor.udp_adv_ip, revogi_infor.udp_remote_port, revogi_infor.udp_local_port,&revogi_infor);
	return ret;
}
//switch udp 接收
static void revogi_rec_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
	uint8_t rec[BUFF_LEN_MAX];
	uint8_t i;
	struct ip_addr ip_addr;
	uint8_t *ptr;
	revogi_infor_t *pSw;
	
	pSw = (revogi_infor_t*)arg;
	printf("[Revogi]: broadcast ip: %X\r\n", (uint32_t)addr->addr);
	memset(rec,0,BUFF_LEN_MAX);
	memcpy(rec, p->payload,p->len);
	revogi_infor.tcp_ip.addr = addr->addr;
//	if((NumofStr(rec, '.')==3)&&(NumofStr(rec, ',')==5))
//	{
//		if(inet_aton(rec, (struct in_addr*)&ip_addr)==0)
//			return;
//		if(ip_addr.addr!=addr->addr)
//			return;
//		revogi_infor.tcp_ip.addr = ip_addr.addr;
//		
//		ptr = strstr((char*)rec, ",");
//		if(ptr==NULL)
//			return;
//		memcpy(revogi_infor.mac, ptr+1,12);
//		ptr = strstr((char*)(ptr+1),",");
//		if(ptr==NULL)
//			return;
//		memcpy(revogi_infor.sn,ptr+1,9);
//		ptr = strstr((char*)(ptr+10),",");
//		if(ptr==NULL)
//			return;
//		revogi_infor.is_online =(bool) *(ptr+1);
//		revogi_infor.state =(bool) *(ptr+3);
//	}
	
	
	
	printf("[sw]: ");
	for(i=0;i<p->len;i++)
	{
		printf(" %X",rec[i] );
	}
	printf("\r\n");

  pbuf_free(p);
}

//switch udp 发送
err_t revogi_find_udp_Send(uint8_t *p, uint16_t len)
{
		err_t ret = ERR_OK;
	
		ret = udp_client_Send(revogi_infor.udp_pcb, revogi_infor.udp_adv_ip, revogi_infor.udp_remote_port, p, len);
		return ret;
}




