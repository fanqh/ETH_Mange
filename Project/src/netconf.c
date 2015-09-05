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

  /* Configure the board opeartin mode: Client/Server */  
  LCD_DisplayStringLine(Line5, "  Keep Key2 button  ");
  LCD_DisplayStringLine(Line6, "pressed to activate ");
  LCD_DisplayStringLine(Line7, "     the Client     ");

//  Delay_ARMJISHU(600*KEY_DELAY);
  //Delay_ARMJISHU(5000*KEY_DELAY);
  
  //if(!STM_EVAL_PBGetState(Button_KEY))
  {	
    macaddress[5]=CLIENTMAC6;
	
	Server = NOT_SELECTED;	
	LCD_DisplayStringLine(Line5, "                    ");
	LCD_DisplayStringLine(Line6, "  Client selected   ");	
	LCD_DisplayStringLine(Line7, "                    ");
//	Delay_ARMJISHU(LCD_DELAY);
  }
//  else
//  {
//        Server = SELECTED;	
//	LCD_DisplayStringLine(Line5, "                    ");
//	LCD_DisplayStringLine(Line6, "  Server selected   ");	
//	LCD_DisplayStringLine(Line7, "                    ");
//	Delay_ARMJISHU(LCD_DELAY);
//  }

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
void Display_Periodic_Handle(__IO uint32_t localtime)
{ 
  uint8_t macaddress[6];
  u16 PHYRegData;
  /* 250 ms */
  if (localtime - DisplayTimer >= LCD_TIMER_MSECS)
  {
    DisplayTimer = localtime;

    /* We have got a new IP address so update the display */
    if (IPaddress != netif.ip_addr.addr)
    {
      __IO uint8_t iptab[4];
      uint8_t iptxt[20];

      /* Read the new IP address */
      IPaddress = netif.ip_addr.addr;

      iptab[0] = (uint8_t)(IPaddress >> 24);
      iptab[1] = (uint8_t)(IPaddress >> 16);
      iptab[2] = (uint8_t)(IPaddress >> 8);
      iptab[3] = (uint8_t)(IPaddress);

      sprintf((char*)iptxt, "   %d.%d.%d.%d    ", iptab[3], iptab[2], iptab[1], iptab[0]);

      /* Display the new IP address */
#if LWIP_DHCP
      if (netif.flags & NETIF_FLAG_DHCP)
      {        
		/* Display the IP address */
		LCD_DisplayStringLine(Line7, "IP address assigned ");
        LCD_DisplayStringLine(Line8, "  by a DHCP server  ");
        LCD_DisplayStringLine(Line9, iptxt);
		  Delay_ARMJISHU(800 * KEY_DELAY);
		/** Start the client/server application: only when a dynamic IP address has been obtained  **/

      }
      else
#endif
      {
        /* Display the IP address */
		LCD_DisplayStringLine(Line8, "  Static IP address   ");
        LCD_DisplayStringLine(Line9, iptxt);	    
		Delay_ARMJISHU(LCD_DELAY);
//        Delay_ARMJISHU(11000*KEY_DELAY);
        ETH_GetMACAddress(0, macaddress);
        printf("\n\r Your MAC are configured: %X:%X:%X:%X:%X:%X", macaddress[0], macaddress[1], macaddress[2], macaddress[3], macaddress[4], macaddress[5]);
        printf("\n\r Static IP address: %s", iptxt);
//        Delay_ARMJISHU(200*KEY_DELAY);
      }           

	    /* Clear the LCD */	   	
        LCD_Clear(Black);
	    LCD_SetBackColor(Black);
        LCD_SetTextColor(White);		
        LCD_DisplayStringLine(Line8, "  www.armjishu.com  ");
        LCD_DisplayWelcomeStr(Line9);        

        /* www.armjishu.com LCD��ʾ����״̬ */
        PHYRegData = ETH_ReadPHYRegister(0,17);
        if(PHYRegData & 0x3000)
        {  
          /* Set Ethernet speed to 10M following the autonegotiation */    
          printf("\n\r==>ETH_Speed_10M!");
          LCD_DisplayStringLine(Line5, "   ETH_Speed_10M    ");
        }
        else
        {   
          /* Set Ethernet speed to 100M following the autonegotiation */ 
          printf("\n\r==>ETH_Speed_100M!");     
          LCD_DisplayStringLine(Line5, "   ETH_Speed_100M   ");
        } 
        /* Configure the MAC with the Duplex Mode fixed by the autonegotiation process */
        if((PHYRegData & 0xA000) != (uint32_t)RESET)
        {
          /* Set Ethernet duplex mode to FullDuplex following the autonegotiation */
          printf("\n\r==>ETH_Mode_FullDuplex!");
          LCD_DisplayStringLine(Line6, " ETH_Mode_FullDuplex");
        }
        else
        {
          /* Set Ethernet duplex mode to HalfDuplex following the autonegotiation */
          printf("\n\r==>ETH_Mode_HalfDuplex!");
          LCD_DisplayStringLine(Line6, " ETH_Mode_HalfDuplex");
        }
        
        	   
        iptab[0] = (uint8_t)(IPaddress >> 24);
        iptab[1] = (uint8_t)(IPaddress >> 16);
        iptab[2] = (uint8_t)(IPaddress >> 8);
        iptab[3] = (uint8_t)(IPaddress);
	    sprintf((char*)iptxt, "ip: %d.%d.%d.%d ", iptab[3], iptab[2], iptab[1], iptab[0]);		

        LCD_DisplayWelcomeStr(Line0);
	    LCD_DisplayStringLine(Line2, iptxt);
        //Delay_ARMJISHU(11000*KEY_DELAY);

        ETH_GetMACAddress(0, macaddress);
        printf("\n\r Your MAC are configured: %X:%X:%X:%X:%X:%X", macaddress[0], macaddress[1], macaddress[2], macaddress[3], macaddress[4], macaddress[5]);
        printf("\n\r Your Ip are configured: %s\n\r", &iptxt[3]);
        
//	    if(Server)
	    {
	      LCD_DisplayStringLine(Line1, "  You are a server  ");
		 
		  /* Initialize the server application */
	      server_init(); 
	    }
//	    else
	    {
	      LCD_DisplayStringLine(Line1, "  You are a client  ");
		  
		  /* Initialize the client application */
	      client_init();
	    }	 

        /* Read the new gw address www.armjishu.com */
        IPaddress = netif.gw.addr;
	    iptab[0] = (uint8_t)(IPaddress >> 24);
        iptab[1] = (uint8_t)(IPaddress >> 16);
        iptab[2] = (uint8_t)(IPaddress >> 8);
        iptab[3] = (uint8_t)(IPaddress);
        sprintf((char*)iptxt, "gw: %d.%d.%d.%d  ", iptab[3], iptab[2], iptab[1], iptab[0]);
        printf(" Your gw are configured: %s\n\r", &iptxt[3]);
        LCD_DisplayStringLine(Line3, iptxt);

        /* �ָ� the new IP address */
        IPaddress = netif.ip_addr.addr;
        
    }

#if LWIP_DHCP
    
    else if (IPaddress == 0)
    {
      /* We still waiting for the DHCP server */
	  LCD_DisplayStringLine(Line4, "     Looking for    ");
      LCD_DisplayStringLine(Line5, "     DHCP server    ");
      LCD_DisplayStringLine(Line6, "     please wait... ");

      LedToggle &= 3;

      if(LedToggle ==0)
      LCD_DisplayWelcomeStr(Line7); 
      else if(LedToggle ==1)
      LCD_DisplayStringLine(Line7, "  ***************** ");
      else if(LedToggle ==2)
      LCD_DisplayStringLine(Line7, "  %%%%%%%%%%%%%%%%% ");
      else if(LedToggle ==3)
      LCD_DisplayStringLine(Line7, "  ################# ");
      
      STM_EVAL_LEDToggle((Led_TypeDef)(LedToggle++));
      
      /* If no response from a DHCP server for MAX_DHCP_TRIES times */
	  /* stop the dhcp client and set a static IP address */
	  if (netif.dhcp->tries > MAX_DHCP_TRIES)
      {
        struct ip_addr ipaddr;
        struct ip_addr netmask;
        struct ip_addr gw;

        LCD_DisplayStringLine(Line7, "    DHCP timeout    ");        

        dhcp_stop(&netif);

        IP4_ADDR(&ipaddr, 192, 168, 1, 6);
        IP4_ADDR(&netmask, 255, 255, 255, 0);
        IP4_ADDR(&gw, 192, 168, 1, 1);

        netif_set_addr(&netif, &ipaddr , &netmask, &gw);

      }
    }
#endif
  } 
}

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
