/**
  ******************************************************************************
  * @file    netconf.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11/20/2009
  * @brief   Network connection configuration
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
#include "lwip/memp.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "ethernetif.h"
#include "main.h"
#include "netconf.h"
#include <stdio.h>
#include <string.h>
#include "eth_mange.h"
#include "broadlink.h"
#include "stm32_eth.h"

/* Private typedef -----------------------------------------------------------*/
#define LCD_DELAY             3000
#define KEY_DELAY 			  3000
#define LCD_TIMER_MSECS       250
#define MAX_DHCP_TRIES        4
#define SELECTED              1
#define NOT_SELECTED		  (!SELECTED)
#define CLIENTMAC6            2
//#define CLIENTMAC6            3
//#define CLIENTMAC6            4

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
struct netif netif;
__IO uint32_t TCPTimer = 0;
__IO uint32_t ARPTimer = 0;

#ifdef LWIP_DHCP
__IO uint32_t DHCPfineTimer = 0;
__IO uint32_t DHCPcoarseTimer = 0;
static uint32_t IPaddress = 0;
#endif

__IO uint32_t DisplayTimer = 0;
uint8_t LedToggle = 4;
uint8_t	Server = 0;

/* Private function prototypes -----------------------------------------------*/
extern void client_init(void);
extern void server_init(void);

device_infor_t Device_Infor;


/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes the lwIP stack
  * @param  None
  * @retval None
  */
void LwIP_Init(void)
{
  struct ip_addr ipaddr;
  struct ip_addr netmask;
  struct ip_addr gw;
  //uint8_t macaddress[6]={0,0,0,0,0,1};
  uint8_t macaddress[6]={0xcc,0xbb,0xaa,0x99,0x88,0x1};

  /* Initializes the dynamic memory heap defined by MEM_SIZE.*/
  mem_init();

  /* Initializes the memory pools defined by MEMP_NUM_x.*/
  memp_init();


#if LWIP_DHCP
  ipaddr.addr = 0;
  netmask.addr = 0;
  gw.addr = 0;

#else
  IP4_ADDR(&ipaddr, 192, 168, 1, 6);
  IP4_ADDR(&netmask, 255, 255, 255, 0);
  IP4_ADDR(&gw, 192, 168, 1, 1);
#endif

  Set_MAC_Address(macaddress);

  /* - netif_add(struct netif *netif, struct ip_addr *ipaddr,
            struct ip_addr *netmask, struct ip_addr *gw,
            void *state, err_t (* init)(struct netif *netif),
            err_t (* input)(struct pbuf *p, struct netif *netif))
    
   Adds your network interface to the netif_list. Allocate a struct
  netif and pass a pointer to this structure as the first argument.
  Give pointers to cleared ip_addr structures when using DHCP,
  or fill them with sane numbers otherwise. The state pointer may be NULL.

  The init function pointer must point to a initialization function for
  your ethernet netif interface. The following code illustrates it's use.*/
  netif_add(&netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);

  /*  Registers the default network interface.*/
  netif_set_default(&netif);


#if LWIP_DHCP
  /*  Creates a new DHCP client for this interface on the first call.
  Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
  the predefined regular intervals after starting the client.
  You can peek in the netif->dhcp struct for the actual DHCP status.*/
  dhcp_start(&netif);
#endif

  /*  When the netif is fully configured this function must be called.*/
  netif_set_up(&netif);

}

/**
  * @brief  Called when a frame is received
  * @param  None
  * @retval None
  */
void LwIP_Pkt_Handle(void)
{
  /* Read a received packet from the Ethernet buffers 
                and send it to the lwIP for handling */
  ethernetif_input(&netif);
}

/**
  * @brief  LwIP periodic tasks
  * @param  localtime the current LocalTime value
  * @retval None
  */
void LwIP_Periodic_Handle(__IO uint32_t localtime)
{

  /* TCP periodic process every 250 ms */
  if ((localtime - TCPTimer) >= TCP_TMR_INTERVAL)
  {
    TCPTimer =  localtime;
    tcp_tmr();
  }
  /* ARP periodic process every 5s */
  if ((localtime - ARPTimer) >= ARP_TMR_INTERVAL)
  {
    ARPTimer =  localtime;
    etharp_tmr();
  }

#if LWIP_DHCP
  /* Fine DHCP periodic process every 500ms */
  if (localtime - DHCPfineTimer >= DHCP_FINE_TIMER_MSECS)
  {
    DHCPfineTimer =  localtime;
    dhcp_fine_tmr();
  }
  /* DHCP Coarse periodic process every 60s */
  if (localtime - DHCPcoarseTimer >= DHCP_COARSE_TIMER_MSECS)
  {
    DHCPcoarseTimer =  localtime;
    dhcp_coarse_tmr();
  }
#endif

}

extern void ETH_GetMACAddress(uint32_t MacAddr, uint8_t *Addr);

/**
  * @brief  LCD & LEDs periodic handling
  * @param  localtime: the current LocalTime value
  * @retval None
  */
uint8_t flag_server=0;
void Display_Periodic_Handle(__IO uint32_t localtime)
{ 
  uint8_t macaddress[6]; 
	
		/* 250 ms */
  if (localtime - DisplayTimer >= LCD_TIMER_MSECS)
  {
    DisplayTimer = localtime;

    /* We have got a new IP address so update the display */
    if (IPaddress != netif.ip_addr.addr)
    {
      /* Read the new IP address */
      IPaddress = netif.ip_addr.addr;					
      /* Display the new IP address */
#if LWIP_DHCP
      if (netif.flags & NETIF_FLAG_DHCP)
      {        
				
				Device_Infor.ip_addr = netif.ip_addr;
				Device_Infor.gw = netif.gw;
				Device_Infor.netmask = netif.gw;
				ETH_GetMACAddress(0, Device_Infor.macaddr);
				Device_Infor.is_connet = 1;
				/** Start the client/server application: only when a dynamic IP address has been obtained  **/

      }
#endif          
        ETH_GetMACAddress(0, macaddress);
        printf("=>Your MAC are configured: %X:%X:%X:%X:%X:%X\r\n", macaddress[0], macaddress[1], macaddress[2], macaddress[3], macaddress[4], macaddress[5]);
				printf("=>Your ip are configured: %d,%d,%d,%d\r\n",(uint8_t)(IPaddress), (uint8_t)(IPaddress >> 8), (uint8_t)(IPaddress >> 16), (uint8_t)(IPaddress >> 24));
				printf("=>Your gw are configured: %d,%d,%d,%d\r\n",(uint8_t)(netif.gw.addr), (uint8_t)(netif.gw.addr >> 8), (uint8_t)(netif.gw.addr >> 16), (uint8_t)(netif.gw.addr >> 24));    
//		  /* Initialize the server application */
				if(flag_server==0)
  	    {
					flag_server = 1;
					server_init();
				}
//		  /* Initialize the client application */
  	      client_init();   
    }

#if LWIP_DHCP
    
    else if (IPaddress == 0)
    {
//			memset(&Device_Infor, 0, sizeof(device_infor_t));
      if((LedToggle&3) ==0)
				printf("=>Looking for DHCP server connectting......\r\n");
			LedToggle++;
      
    }
		else
		{
		}
#endif
//		if((Device_Infor.is_connet==1)&&(broadlink_infor.is_connect==0))
//		{
//			broadlink_infor.is_connect=1;
//			broadlink_broadcast_init();
//		}
		
  } 
}

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
