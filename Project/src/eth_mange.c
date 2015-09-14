#include "main.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>

#define SMARTSW_UDP_BROADCAST_IP   			255.255.255.255
#define SMARTSW_UDP_PORT					 			48899
#define SMARTSW_IP_PORT									8899
#define DEVICE_LOACAL_UDP_PORT				 	48899


static struct tcp_pcb *TcpPCB;
//智能开关的广播指令
const uint8_t SmartSwBroadcastCmd[] = "YZ-RECOSCAN";
static void udp_client_SmartSW_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
static void tcp_SmartSW_errf(void);
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
void udp_Broacast_TO_SmartSw(void)
{
   struct udp_pcb *upcb;
   struct pbuf *p;
	struct ip_addr ip_udp_server;

   SET_IP4_ADDR(&ip_udp_server, SMARTSW_UDP_BROADCAST_IP);                              
   /* Create a new UDP control block  */
   upcb = udp_new();   
   p = pbuf_alloc(PBUF_TRANSPORT, sizeof(Sent), PBUF_RAM);
	 p->payload = (void*)SmartSwBroadcastCmd; 
	 upcb->local_port = DEVICE_LOACAL_UDP_PORT;
	 udp_connect(upcb, &ip_udp_server, SMARTSW_UDP_PORT);	 
	 udp_send(upcb, p); 
	   /* Reset the upcb */
   udp_disconnect(upcb);
   
   /* Bind the upcb to any IP address and the UDP_PORT port*/
   udp_bind(upcb, IP_ADDR_ANY, SMARTSW_UDP_PORT);
	
	
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
  * @retval None client
  */
static void udp_client_SmartSW_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
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

  /* Create a new TCP control block  */
  pcb = tcp_new();
  /* Assign to the new pcb a local IP address and a port number */
  tcp_bind(pcb, IP_ADDR_ANY, SMARTSW_IP_PORT);

  /* Connect to the server: send the SYN */
  tcp_connect(pcb, addr, TCP_PORT, tcp_client_connected);
	tcp_err(pcb, tcp_SmartSW_errf);
	
  /* Free the p buffer */
  pbuf_free(p);
}


static void tcp_SmartSW_errf(void *arg,err_t err)
{
	
}

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





