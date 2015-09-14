/**
  ******************************************************************************
  * @file    client.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11/20/2009
  * @brief   A sample UDP/TCP client
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
#include "lwip/ip_addr.h"
#include <string.h>
#include <stdio.h>


/* Private typedef -----------------------------------------------------------*/
#define UDP_SERVER_PORT      9001
#define UDP_CLIENT_PORT      9002
#define TCP_PORT      4
//#define UDP_SERVER_IP				192,168,0,103
#define UDP_SERVER_IP				255,255,255,255


#define UDP_LOCAL_IP        192,168,0,101                                                                                                                                                                                                                        


/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static struct tcp_pcb *TcpPCB;

/* Private function prototypes -----------------------------------------------*/
void udp_client_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
void tcp_client_err(void *arg, err_t err);
err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);

err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

/* Private functions ---------------------------------------------------------*/
struct ip_addr ip_udp_server;


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
//		uint8_t Sent[]="fanqh test udp client\r\n";

uint8_t Sent[]="fanqh test udp client\r\n";
uint8_t sm[] = {0xff,0xee,0x11, 0x00,0x00,0x11,0xef,0xfe};
void client_init(void)
{
   struct udp_pcb *upcb;
   struct pbuf *p;

   SET_IP4_ADDR(&ip_udp_server, UDP_SERVER_IP);                              
   /* Create a new UDP control block  */
   upcb = udp_new();   
   p = pbuf_alloc(PBUF_TRANSPORT, sizeof(Sent), PBUF_RAM);
	 p->payload = (void*)sm; 
	p->len =8;
	p->tot_len = 8;
	 upcb->local_port = UDP_CLIENT_PORT;
	 udp_connect(upcb, &ip_udp_server, UDP_SERVER_PORT);	 
	 udp_send(upcb, p); 
	   /* Reset the upcb */
   udp_disconnect(upcb);
   
   /* Bind the upcb to any IP address and the UDP_PORT port*/
   udp_bind(upcb, IP_ADDR_ANY, UDP_CLIENT_PORT);
	
	
   udp_recv(upcb, udp_client_callback, NULL);

   /* Free the p buffer */
   pbuf_free(p);
  
}

/**
  * @brief  This function is called when a datagram is received
   * @param arg user supplied argument (udp_pcb.recv_arg)
   * @param upcb the udp_pcb which received data
   * @param p the packet buffer that was received
   * @param addr the remote IP address from which the packet was received
   * @param port the remote port from which the packet was received
  * @retval None
  */

/*
* void tcp_err(struct tcp_pcb *pcb, void (* errf)(void *arg, err_t err)) 
* void tcp_arg(struct tcp_pcb *pcb, void *arg) 
*/
uint16_t len1, len2;
uint8_t rec[256];
void udp_client_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
  struct tcp_pcb *pcb;
  __IO uint8_t iptab[4];
  uint8_t iptxt[20];
	
	
  /* Read the Server's IP address */
  iptab[0] = (uint8_t)((uint32_t)(addr->addr) >> 24);  
  iptab[1] = (uint8_t)((uint32_t)(addr->addr) >> 16);
  iptab[2] = (uint8_t)((uint32_t)(addr->addr) >> 8); 
  iptab[3] = (uint8_t)((uint32_t)(addr->addr));

  sprintf((char*)iptxt, "is: %d.%d.%d.%d     ", iptab[3], iptab[2], iptab[1], iptab[0]);
	
  LCD_DisplayStringLine(Line3, "The server's IP add.");
  LCD_DisplayStringLine(Line4, iptxt);

	len1 = p->tot_len;
	len2 = p->len;
	memcpy(rec, p->payload, len1);
  /* Create a new TCP control block  */
  pcb = tcp_new();

  /* Assign to the new pcb a local IP address and a port number */
  tcp_bind(pcb, IP_ADDR_ANY, TCP_PORT);

  /* Connect to the server: send the SYN */
  tcp_connect(pcb, addr, TCP_PORT, tcp_client_connected);
	
 
//    tcp_accept(pcb, APP_accept);  //
 //   tcp_sent(pcb, App_sent);
  /* Free the p buffer */
  pbuf_free(p);
}



/*



    Pcb1 = tcp_new();
    tcp_bind(Pcb1, &ulIPAddr, ulPORT) ;
    tcp_arg(Pcb1, NULL);
    tcp_recv(Pcb1, APP_recv);
 
    tcp_accept(Pcb1, APP_accept);  //
    tcp_sent(Pcb1, App_sent);
    tcp_poll(Pcb1, TCP_SER_Poll, 4);



*/
/**
  * @brief  This function is called when the connection with the remote 
  *         server is established
  * @param arg user supplied argument
  * @param tpcb the tcp_pcb which received data
  * @param err error value returned by the tcp_connect 
  * @retval error value
  */
	
	
err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
  LCD_DisplayStringLine(Line5, "Led control started ");
  
  /* Display Leds Control blocks */
  LCD_SetTextColor(Blue);
  LCD_DrawRect(180, 310, 40, 60);
  LCD_SetTextColor(Red);
  LCD_DrawRect(180, 230, 40, 60);
  LCD_SetTextColor(Yellow);
  LCD_DrawRect(180, 150, 40, 60);
  LCD_SetTextColor(Green);
  LCD_DrawRect(180, 70, 40, 60);

  TcpPCB = tpcb;
	tcp_write(TcpPCB, Sent, sizeof(Sent), 1);
  tcp_recv(tpcb, tcp_client_recv);  
  return ERR_OK;
}

err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	printf("some");
	pbuf_free(p);
	return ERR_OK;
}


/**
  * @brief  Send to the server the led that should be toogled.
  * @param  Led the led that should be toogled
  * @retval None
  */
void tcp_led_control(Led_TypeDef Led)
{
  char ledstatus;

  ledstatus = (char) Led;
  
  tcp_write(TcpPCB, &ledstatus, sizeof(ledstatus), 1);

  /* send the data right now */
  tcp_output(TcpPCB); 
}

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
