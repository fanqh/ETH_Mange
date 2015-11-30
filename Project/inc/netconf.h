																			   /**
  ******************************************************************************
  * @file    netconf.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11/20/2009
  * @brief   This file contains all the functions prototypes for the netconf.c 
  *          file.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __NETCONF_H
#define __NETCONF_H
#include "stm32f10x.h"
#include "netif.h"
#include "device_server.h"

#ifdef __cplusplus
 extern "C" {
#endif
	 
#define MANAGE_UDP_SERVER_PORT  9009	 
#define DEBUG printf
	 
typedef enum
{
	S_IDLE,
	S_CONNECTING,
	S_CONNECTED,
	S_CLOSING,
	S_CLOSED,
	S_SEND,
	S_RECEIVE,
	
}tcp_state_t;	

typedef struct
{
	uint16_t retry;
	tcp_state_t tstate;
	struct ip_addr  tip;
	struct tcp_pcb  *tpcb;
	uint16_t tlocal_port;
	uint16_t tremote_port;
	err_t (* recv)(struct tcp_pcb *tpcb,struct pbuf *p,void *arg, err_t err);
	uint8_t mac[12];	
}tcp_struct_t;

typedef struct
{
	struct ip_addr  udp_adv_ip;
	struct udp_pcb  *upcb;
	uint16_t uremote_port;
	uint16_t ulocal_port;
}udp_struct_t;
	 
typedef struct
{
	struct netif *pnetif;
	uint8_t macaddr[6];
	uint8_t ConnectState;
	uint8_t tcp_num;
	uint8_t udp_num;
	Dev_Server_infor_t *server;
	
}device_infor_t; 	 

extern device_infor_t Device_Infor;
/* Includes ------------------------------------------------------------------*/
void LwIP_Init(void);
void LwIP_Pkt_Handle(void);
void LwIP_Periodic_Handle(__IO uint32_t localtime);
void Display_Periodic_Handle(__IO uint32_t localtime);


#ifdef __cplusplus
}
#endif

#endif /* __NETCONF_H */


/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/

