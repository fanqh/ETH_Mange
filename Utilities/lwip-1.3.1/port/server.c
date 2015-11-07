/**
  ******************************************************************************
  * @file    server.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11/20/2009
  * @brief   A sample UDP/TCP server application.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>
#include "netconf.h"
#include "broadlink.h"
#include "smart_switch.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
//#define UDP_SERVER_PORT    8   /* define the UDP local connection port */
//#define UDP_CLIENT_PORT    4   /* define the UDP remote conne                                                                                        
#define TCP_PORT    4	/* define the TCP connection port */
const char FindCMD[]="action:find,device:";
const char maccmd[] = "mac:";
const char ipcmd[]="IP:";
const char FindCMDResp[] = {'c','m','d',':',0,0,0,0,0,0,'I','P',':',0,0,0,0};
struct FindCMDResp_t
{
	char maccmd[4];
	uint8_t macaddr[6];
	char ipcmd[3];
	struct ip_addr local_ip;
}find_resp;

		smart_switch_infor_t  pSwitch_infor;

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void udp_server_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
err_t tcp_server_accept(void *arg, struct tcp_pcb *pcb, err_t err);
static err_t tcp_server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initialize the server application.
  * @param  None
  * @retval None
  */
void server_init(void)
{
   struct udp_pcb *upcb;                                 
   
   upcb = udp_new();
   if(udp_bind(upcb, IP_ADDR_ANY, MANAGE_UDP_SERVER_PORT)==ERR_OK)
			udp_recv(upcb, udp_server_callback, NULL);  
	 else
		 DEBUG("[server]: add and port is used\r\n");
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
void udp_server_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
	uint8_t buff[256];
	uint8_t *ptr,*pGW;
	struct ip_addr remote_gw;
	
  
	if(p->len>256)
		return;
	memcpy(buff,p->payload,p->len);
	buff[p->len] = '\0';
	DEBUG("\r\n[server]: receive client ip addr: %d.%d,%d,%d\r\n\
	port: %d\r\n", (uint8_t)((uint32_t)(addr->addr)),(uint8_t)((uint32_t)(addr->addr) >> 8), (uint8_t)((uint32_t)(addr->addr) >> 16), (uint8_t)((uint32_t)(addr->addr) >> 24),port);
  DEBUG("[server]: receive: %s\r\n", buff);
	
	
	ptr = (uint8_t*)strstr((char*)buff, FindCMD);
	if(ptr)
	{
		DEBUG("[server]: RemoteGw: %s\r\n",ptr+sizeof(FindCMD)-1); 
		pGW = ptr+sizeof(FindCMD)-1;
		remote_gw.addr = inet_addr(pGW);
		if(remote_gw.addr == Device_Infor.gw.addr)
		{
			struct pbuf *pudpSend;
			
			strcat(find_resp.maccmd, "mac:");
			strcat(find_resp.ipcmd,"IP:");
			find_resp.local_ip = Device_Infor.ip_addr;
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
		switch_udp_Send("me", 3);
		printf("[server]: sending\r\n");
	}	
	else if(strstr((char*)buff,"connect"))
	{
		pSwitch_infor.tcp_ip.addr  = 0x6600A8C0; 
		Switch_TCP_Client_Attemp_Connect(&pSwitch_infor);
		
	}
	else if(strstr((char*)buff,"tcp"))
	{
		Switch_TCP_Send(&pSwitch_infor, "fanqh", 5);
	}
  pbuf_free(p);
   
}

/**
  * @brief  This funtion is called when a TCP connection has been established on the port TCP_PORT.
  * @param  arg	user supplied argument 
  * @param  pcb	the tcp_pcb which accepted the connection
  * @param  err error value
  * @retval ERR_OK
  */
err_t tcp_server_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{ 
  /* Specify the function that should be called when the TCP connection receives data */
  tcp_recv(pcb, tcp_server_recv);

  return ERR_OK;  
}

/**
  * @brief  This function is called when a data is received over the TCP_PORT.
  *         The received data contains the number of the led to be toggled.
  * @param  arg	user supplied argument 
  * @param  pcb	the tcp_pcb which accepted the connection
  * @param  p the packet buffer that was received
  * @param  err error value
  * @retval ERR_OK
  */
static err_t tcp_server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
  char data;

  /* Read the led number */
  data = *(((char *)p->payload));  

  /* Toggle the specified led */
  switch (data)
  {
  	case LED1:
	    STM_EVAL_LEDToggle(LED1);
		break;
	
	case LED2:
	    STM_EVAL_LEDToggle(LED2);
		break;
	
	case LED3:
	    STM_EVAL_LEDToggle(LED3);
		break;
	
	case LED4:
	    STM_EVAL_LEDToggle(LED4);
		break;
	
	default:
		break;
  }

  /* Free the p buffer */
  pbuf_free(p);

  return ERR_OK;
}

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
