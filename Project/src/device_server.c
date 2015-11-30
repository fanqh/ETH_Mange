
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>
#include "broadlink.h"
#include "smart_switch.h"
#include "revogi.h"
#include "udp_client.h"
#include "device_server.h"

/* Private typedef -----------------------------------------------------------*/
struct FindCMDResp_t
{
	char maccmd[4];
	uint8_t macaddr[6];
	char ipcmd[3];
	struct ip_addr local_ip;
}find_resp;


/* Private define ------------------------------------------------------------*/                                                                                   
#define TCP_PORT    4	/* define the TCP connection port */
const char FindCMD[]="action:find,device:";
const char maccmd[] = "mac:";
const char ipcmd[]="IP:";
const char FindCMDResp[] = {'c','m','d',':',0,0,0,0,0,0,'I','P',':',0,0,0,0};

//		smart_switch_infor_t  pSwitch_infor;
uint8_t SwitchAdvCMD[] = "YZ-RECOSCAN";
uint8_t Switch_RealTimeCMD[] = "AT+YZSWITCH=1,ON,201511222133\r\n";

//smart Power trip
uint8_t TripFindCMD[] = "00sw=all,2015-11-27,21:16:39,+8";
uint8_t TripTurnoffCMD[] = "GET /?cmd=200&json={\"sn\":\"SWW6012003000015\",\"port\":0,\"state\":1} HTTP/1.1\r\nHost: 192.168.0.103";


/* Private function prototypes -----------------------------------------------*/
static void udp_server_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);

Dev_Server_infor_t dev_Server_inf;




Dev_Server_infor_t* GetDev_server(void)
{
	return &dev_Server_inf;
}

/**
  * @brief  Initialize the server application.
  * @param  None
  * @retval None
  */
err_t udp_server_init(void *pd)
{
    err_t ret;
	struct udp_pcb *upcb;                                 

	upcb = udp_new();
	if(upcb==NULL)
		return ERR_BUF;
	if((ret = udp_bind(upcb, IP_ADDR_ANY, MANAGE_UDP_SERVER_PORT))!=ERR_OK)
	{		
		udp_remove(upcb);
		return ret;
	}
	dev_Server_inf.arg = pd;
	dev_Server_inf.upcb_server.upcb = upcb;
//	pd->udp_num++;
	udp_recv(upcb, udp_server_callback, &dev_Server_inf);  
	return ret;
}

/**
  * @brief This function is called when an UDP datagrm has been received on the port UDP_PORT.
  * @param arg user supplied argument (udp_pcb.recv_arg)
  * @param pcb the udp_pcb which received data
  * @param p the packet buffer that was received
  * @param addr the remote IP address from which the packet was received
  * @param port the remote port from which the packet was received
  * @retval None
  */
static void udp_server_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
	uint8_t buff[256];
	uint8_t *ptr,*pGW;
	struct ip_addr remote_gw;
	Dev_Server_infor_t *ps;

	if(p->len>256)
		return;
	memcpy(buff,p->payload,p->len);
	buff[p->len] = '\0';
	DEBUG("\r\n[server]: receive client ip addr: %d.%d,%d,%d\r\n\
	port: %d\r\n", (uint8_t)((uint32_t)(addr->addr)),(uint8_t)((uint32_t)(addr->addr) >> 8), (uint8_t)((uint32_t)(addr->addr) >> 16), (uint8_t)((uint32_t)(addr->addr) >> 24),port);
	DEBUG("[server]: receive: %s\r\n", buff);

	
	ps = (Dev_Server_infor_t*) arg;
	if(ps->upcb_server.addr.addr!=addr->addr)
		ps->upcb_server.addr.addr = addr->addr;
	if(ps->upcb_server.port!=port)
		ps->upcb_server.port = port;
	ptr = (uint8_t*)strstr((char*)buff, FindCMD);
	if(ptr)
	{
		DEBUG("[server]: RemoteGw: %s\r\n",ptr+sizeof(FindCMD)-1); 
		pGW = ptr+sizeof(FindCMD)-1;
		remote_gw.addr = inet_addr(pGW);
		if(remote_gw.addr == Device_Infor.pnetif->gw.addr)
		{
			struct pbuf *pudpSend;
			
			strcat(find_resp.maccmd, "mac:");
			strcat(find_resp.ipcmd,"IP:");
			find_resp.local_ip = Device_Infor.pnetif->ip_addr;
			memcpy(find_resp.macaddr , Device_Infor.macaddr, 6);
		
			pudpSend = pbuf_alloc(PBUF_TRANSPORT, 64, PBUF_RAM);
			pudpSend->payload = (uint8_t*)&find_resp;
			pudpSend->tot_len = pudpSend->len = sizeof(struct FindCMDResp_t);
			upcb->local_port = MANAGE_UDP_SERVER_PORT;
			
			udp_connect(upcb, addr, port);
			udp_send(upcb, pudpSend);
			udp_disconnect(upcb);
		
			pbuf_free(pudpSend);
			udp_bind(upcb, IP_ADDR_ANY, MANAGE_UDP_SERVER_PORT); 
			udp_recv(upcb, udp_server_callback, NULL);  
			
		}
			
	}
	else if(strstr((char*)buff, "find"))
	{
		uint8_t pw[2] = {0,0};
		Broadlink_Find(pw);
		printf("[server]: find\r\n");
	}
	else if(strstr((char*)buff,"keepalive"))
	{
		Broadlink_KeepAlive();
		printf("[server]: keepalive\r\n");
	}
	else if(strstr((char*)buff, "query"))
	{ 
		uint8_t CheckState = 0xf0;
		uint8_t Port = 0;
		uint8_t id[4] = {0,0,0,0};
		
		Broadlink_Query(CheckState, Port, id );
		printf("[server]: query\r\n");
	}
	else if(strstr((char*)buff,"send"))
	{
		switch_udp_Send(SwitchAdvCMD, sizeof(SwitchAdvCMD)-1);
		printf("[server]: sending\r\n");
	}	
	else if(strstr((char*)buff,"connect"))
	{
		//pSwitch_infor.tcp_ip.addr  = 0x6600A8C0; 
	//		pSwitch_infor.tcp_ip.addr  = 0xf801010A;
		Switch_TCP_Client_Attemp_Connect(&switch_infor);
		
	}
	else if(strstr((char*)buff,"tcp"))
	{
		err_t ret;
		ret = Switch_TCP_Send(&switch_infor, Switch_RealTimeCMD, sizeof(Switch_RealTimeCMD));
		if(ret!=ERR_OK)
			printf("send err\r\n");
	}
	else if(strstr((char*)buff, "trip"))
	{
		revogi_find_udp_Send(TripFindCMD, sizeof(TripFindCMD)-1);
	}
	else if(strstr((char*)buff, "hello"))
	{
		PowerTrip_TCP_Client_Attemp_Connect(&revogi_infor);
	}
	else if(strstr((char*)buff, "hi"))
	{
		PowerTrip_TCP_Send(&revogi_infor, TripTurnoffCMD, sizeof(TripTurnoffCMD)-1);
	}
	udp_client_Send(upcb, *addr, port, p->payload, p->len);
	pbuf_free(p);
   
}


