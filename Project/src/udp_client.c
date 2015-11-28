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


typedef struct
{
	uint8_t flag;
	uint32_t Timeout;
	struct udp_pcb  *upcb;
}upcb_manage_t;

upcb_manage_t upcb_manage_list[MEMP_NUM_UDP_PCB];  //udp 


upcb_manage_t* GetFreeUpcbList(void)
{
    uint8_t i;
	for(i=0; i<MEMP_NUM_UDP_PCB; i++)
	{
	    if(upcb_manage_list[i].Timeout==0)		
			return &upcb_manage_list[i];
	}
	return NULL;
}

void PushUpcbList(upcb_manage_t* upcb_mange, struct udp_pcb  *upcb, uint32_t timeout)
{
    upcb_mange->Timeout = timeout;
	upcb_mange->upcb = upcb;
}
void FreeUpcbList(uint8_t i)
{
	if(i>MEMP_NUM_UDP_PCB)
		return;
	upcb_manage_list[i].flag = 0;
	upcb_manage_list[i].Timeout = 0;
    udp_remove(upcb_manage_list[i].upcb);			
}
int Find_And_FreeUpcb(struct udp_pcb  *upcb)
{
	uint8_t i;
	
	for(i=0; i<MEMP_NUM_UDP_PCB; i++)
	{
		if(upcb_manage_list[i].upcb==upcb)
		{
			FreeUpcbList(i);
			return 1;
		}	
	}
	return 0;
}

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

err_t udp_client_Send(struct ip_addr ip_addr, uint16_t port, uint8_t *p, uint16_t len, 
						uint16_t local_port,uint32_t res_timeout, void *arg)
{
	struct pbuf *pSend;
	err_t ret = ERR_OK;
	Dev_Server_infor_t *pdev;
	upcb_manage_t *upcb_manage;
	struct udp_pcb  *upcb
    
	if(arg==NULL)
		return ERR_BUF;
	pdev = (Dev_Server_infor_t*)arg;
	pSend = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
	if(pSend==NULL)
		return ERR_BUF;
	
	if(pdev->pDevInfor->udp_num >= MEMP_NUM_UDP_PCB)   //如果udp个数超出系统设置范围，返回err
		return ERR_MEM;
	if((udp_pcb= udp_new())==NULL)
		return ERR_BUF;
	
	if((upcb_manage = GetFreeUpcbList())!=NULL) 
	{
		PushUpcbList(upcb_mange, udp_pcb, res_timeout);
		pdev->pDevInfor->udp_num ++;
	}
	else  //管理列表个数超出
		return ERR_MEM;

	pSend->payload = p;
	pSend->tot_len = pSend->len = len;
	udp_bind(upcb, IP_ADDR_ANY, local_port); //如果已经绑定了此port和地址，不知道连接之后，，本地port是多少。需要验证
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

