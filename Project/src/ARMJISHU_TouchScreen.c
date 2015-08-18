/******************** (C) COPYRIGHT 2009 www.armjishu.com ************************
* File Name          : ARMJISHU_TouchScreen_ADS7843.c
* Author             : www.armjishu.com Team
* Version            : V3.0.1
* Date               : 03/20/2010
* Description        : 
                        ADS7843_CS   is PB1
                        ADS7843_INT  is PC1
*******************************************************************************/
#include "ARMJISHU_TouchScreen.h"
#include "stm32f10x.h"
#include <stdio.h>
#include "stm32f10x_exti.h"


extern void LCD_SetPoint(uint8_t Xpos, uint16_t Ypos, uint16_t Color);

// A/D ͨ��ѡ�������ֺ͹����Ĵ���
#define	CHX 	0x90//0x90 	//ͨ��Y+��ѡ�������	//0x94
#define	CHY 	0xD0//0xd0	//ͨ��X+��ѡ�������	//0xD4

//#define ADS7843_WrCmd  SPI1_SendByte
//#define ADS7843_Read   SPI1_RecvByte

void ADS7843_CS_config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  /* Enable GPIOB, GPIOC and AFIO clock */
  RCC_APB2PeriphClockCmd(RCC_ADS7843_CS , ENABLE);  //RCC_APB2Periph_AFIO
  
  /* LEDs pins configuration */
  GPIO_InitStructure.GPIO_Pin = GPIO_ADS7843_CS;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIO_ADS7843_CS_PORT, &GPIO_InitStructure);
}

static void ADS7843_INT_config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  /* Enable GPIOB, GPIOC and AFIO clock */
  RCC_APB2PeriphClockCmd(RCC_ADS7843_INT , ENABLE);  //RCC_APB2Periph_AFIO
  
  /* LEDs pins configuration */
  GPIO_InitStructure.GPIO_Pin = GPIO_ADS7843_INT;
  //GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIO_ADS7843_INT_PORT, &GPIO_InitStructure);
}

static void ADS7843_INT_EXIT_Init(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;

    /* Connect Button EXTI Line to Button GPIO Pin */
    GPIO_EXTILineConfig(GPIO_ADS7843_EXTI_PORT_SOURCE, GPIO_ADS7843_EXTI_PIN_SOURCE);  

    /* Configure Button EXTI line */
    EXTI_InitStructure.EXTI_Line = GPIO_ADS7843_EXTI_LINE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

}


/*******************************************************************************
* Function Name  : InterruptConfig
* Description    : Configures the used IRQ Channels and sets their priority.NVIC_Configuration
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void ADS7843_InterruptConfig(void)
{ 
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* Set the Vector Table base address at 0x08000000 */
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000);
  
  /* Configure the Priority Group to 2 bits */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

  /* Enable the EXTI5 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = GPIO_ADS7843_EXTI_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}


static void LCD_BIG_POINT(u16 xScreen, u16 yScreen)
 {
   LCD_SetPoint(xScreen, yScreen, 0xF81F);//Magenta
   LCD_SetPoint(xScreen-1, yScreen, 0xF81F);
   LCD_SetPoint(xScreen+1, yScreen, 0xF81F);
   LCD_SetPoint(xScreen, yScreen-1, 0xF81F);
   LCD_SetPoint(xScreen, yScreen+1, 0xF81F);
   LCD_SetPoint(xScreen-1, yScreen-1, 0xF81F);
   LCD_SetPoint(xScreen-1, yScreen+1, 0xF81F);
   LCD_SetPoint(xScreen+1, yScreen-1, 0xF81F);
   LCD_SetPoint(xScreen+1, yScreen+1, 0xF81F);
 }
 
static void LCD_SMALL_POINT(u16 xScreen, u16 yScreen)
 {
   LCD_SetPoint(xScreen, yScreen, 0xF81F);//Magenta
   LCD_SetPoint(xScreen-1, yScreen, 0xF81F);
   LCD_SetPoint(xScreen-1, yScreen-1, 0xF81F);
   LCD_SetPoint(xScreen, yScreen-1, 0xF81F);
 }
/*=====================================================================*/
u16 _AD2Y(u16 adx) //240
{
  u16 sx=0;
  int r = adx - 200;
  r *= 240;
  sx=r / (4000 - 280);
  if (sx<=0 || sx>240)
    return 0;
  return sx;
}


u16 _AD2X(u16 ady) //320
{
  u16 sy=0;
  int r = ady - 260;
  r *= 320;
  sy=r/(3960 - 360);
  if (sy<=0 || sy>320)
    return 0;
  return sy;
}

/*=====================================================================*/
/*=====================================================================*/
/*=====================================================================*/
/*=====================================================================*/
// A/D ͨ��ѡ�������ֺ͹����Ĵ���
//#define	CHX 	0x90 	//ͨ��Y+��ѡ�������	//0x94
//#define	CHY 	0xd0	//ͨ��X+��ѡ�������	//0xD4



//��ADS7843��IO���ƿ�
	
/*=====================================================================*/

void ADS7843_Init(void)
{
    ADS7843_CS_config();     // ʹ��LCD
    ADS7843_CS_HIGH() ;     // �ر�LCD
    SPI1_Config();
    SPI1_Init_For_Byte();
    SPI1_MOSI_HIGH();
    SPI1_SCK_LOW();
    ADS7843_INT_config();
#if 0
    ADS7843_INT_EXIT_Init();
    ADS7843_InterruptConfig();
#endif
}
/*=====================================================================*/
/*=====================================================================*/


void SPI1_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
    
  // Set as Output push-pull - SCK and MOSI
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  
  GPIO_InitStructure.GPIO_Pin = SPI1_SCK | SPI1_MOSI;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(SPI1_PORT, &GPIO_InitStructure);
  
  //SPI_MISO
  GPIO_InitStructure.GPIO_Pin = SPI1_MISO;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(SPI1_PORT, &GPIO_InitStructure);

}

/*=====================================================================*/
void SPI1_Init_For_Byte(void)
{

}

/*=====================================================================*/
static void ADS7843_SPI_Start( void )
{
    ADS7843_CS_HIGH();
    SPI1_MOSI_HIGH();
    SPI1_SCK_LOW();
    ADS7843_CS_LOW();	      		//оƬ����
}

/*=====================================================================*/
static void SPI_MOSI(u8 data)
{
    if(data)
        SPI1_MOSI_HIGH();
    else
        SPI1_MOSI_LOW();
}

/*=====================================================================*/
/*
#define ShortDelayTime 1
#define LongDelayTime (ShortDelayTime*2)
SPI1_Delay_Short()
{
    __IO uint32_t nCount;
    
    for( nCount= 0; nCount < ShortDelayTime; nCount++ );
}
*/
/*=====================================================================*/


/*=====================================================================*/
static void ADS7843_WrCmd(u8 cmd)
{
    u8 buf, i;

    for( i = 0; i < 8; i++ )
    {
        buf = (cmd >> (7-i)) & 0x1 ;	//MSB��ǰ,LSB�ں�
        SPI_MOSI(buf);	 	//ʱ������������DIN
        SPI1_SCK_HIGH();			//ʱ�����壬һ��8��
        SPI1_SCK_LOW();			//��ʼ����������
    }
	
}

/*=====================================================================*/
static u16 ADS7843_Read(void)
{
	u16 buf = 0 ;
	u8 i;

    SPI1_SCK_HIGH(); //wait busy
	SPI1_SCK_LOW();	
	for( i = 0; i < 12; i++ )
	{
		buf = buf << 1 ;
		SPI1_SCK_HIGH();
		if ( SPI1_MISO_READ() )	
		{
			buf = buf + 1 ;
		}
		SPI1_SCK_LOW();			
	}

	for( i = 0; i < 3; i++ )
	{
		SPI1_SCK_HIGH();
		SPI1_SCK_LOW();			
	}
    
	return( buf ) ;
}

/*=====================================================================*/
#define  READ_TIMES  10
#define  VALID_TIMES  (((READ_TIMES-1)/2) -1)
static u8 ADS7843_Rd_Addata(u16 *X_Addata,u16 *Y_Addata)
{

	u16		i,j,k,x_dataarray[READ_TIMES],y_dataarray[READ_TIMES];
    

        
	for(i=0;(GPIO_ADS7843_INT_VALID)&&(i<READ_TIMES);i++)	//�������.
	{
	    ADS7843_SPI_Start();
		ADS7843_WrCmd( CHX ) ;
		x_dataarray[i] = ADS7843_Read() ;
        ADS7843_CS_HIGH(); 

	    ADS7843_SPI_Start();
		ADS7843_WrCmd( CHY ) ;
		y_dataarray[i] = ADS7843_Read() ;
        ADS7843_CS_HIGH(); 
	}
    
    if(i == READ_TIMES) //һ��Ҫ����times������,������
    {
        // �˲��������ȥ�����ֵ����Сֵ
    	for(i=0;i<READ_TIMES;i++)
    	{
        	for(j=READ_TIMES;j<READ_TIMES-1;j++)
        	{
               if(x_dataarray[j] > x_dataarray[i])
                {
                    k = x_dataarray[j];
                    x_dataarray[j] = x_dataarray[i];
                    x_dataarray[i] = k;
                }
             }
            }  

        // �˲��������ȥ�����ֵ����Сֵ
    	for(i=0;i<READ_TIMES;i++)
    	{
        	for(j=READ_TIMES;j<READ_TIMES-1;j++)
        	{
               if(y_dataarray[j] > y_dataarray[i])
                {
                    k = y_dataarray[j];
                    y_dataarray[j] = y_dataarray[i];
                    y_dataarray[i] = k;
                }
             }
        }
    	// �˲���ȥ�����ֵ����Сֵ
    	*X_Addata = (x_dataarray[VALID_TIMES] + x_dataarray[VALID_TIMES+1] 
                        + x_dataarray[VALID_TIMES+2] + x_dataarray[VALID_TIMES+3]) >> 2;
#ifdef  SIZE_2_8INCH_TFT_LCD
    	*Y_Addata = 4095 - ((y_dataarray[VALID_TIMES] + y_dataarray[VALID_TIMES+1] 
    	                + y_dataarray[VALID_TIMES+2] + y_dataarray[VALID_TIMES+3]) >> 2);
#else   // SIZE_3_2INCH_TFT_LCD
        *Y_Addata= (y_dataarray[VALID_TIMES] + y_dataarray[VALID_TIMES+1] 
    	                + y_dataarray[VALID_TIMES+2] + y_dataarray[VALID_TIMES+3]) >> 2;
#endif

        //printf("\n\r (0x%x, 0x%x), (%d, %d)", *X_Addata, *Y_Addata, *X_Addata, *Y_Addata);
        return 1;
    }
    return 0;
}

/*=====================================================================*/
/*=====================================================================*/
/*=====================================================================*/
u16 distence(u16 data1,u16 data2)
{
    if((data2 > data1 + 2) || (data1 > data2 + 2))
    {
        return 0;
    }

    return 1;    
}

/*=====================================================================*/
void ARMJISHU_TouchScreen(void)
{
  u16 xdata, ydata;
  u32 xScreen, yScreen;

    static u16 sDataX,sDataY;
    
 // printf("\n\r ARMJISHU_TouchScreen_ADS7843 \n\r");
  if(!ADS7843_Rd_Addata(&xdata, &ydata))
  {
     return;
  }
  xScreen = _AD2X(xdata);
  yScreen = _AD2Y(ydata);

   //xScreen = 320 - ((ydata*320)>>12);
   //yScreen = (xdata*240)>>12;

 
  //printf("\n\r (0x%x, 0x%x), (%d, %d)", xdata, ydata, xScreen, yScreen);
  if((xScreen>1)&&(yScreen>1)&&(xScreen<320-1)&&(yScreen<240-1))
  {
       //printf("\n\r%d,%d", xScreen, yScreen);
      if((GPIO_ADS7843_INT_VALID) && distence(sDataX,xScreen) && distence(sDataY,yScreen))
        {
             //LCD_BIG_POINT(yScreen, xScreen);
             LCD_SMALL_POINT(yScreen, xScreen);
        }
      sDataX = xScreen;
      sDataY = yScreen;
  }
    
}
/*=====================================================================*/
/*=====================================================================*/
/*=====================================================================*/
/*=====================================================================*/
/*=====================================================================*/
/*=====================================================================*/
/*=====================================================================*/
/*=====================================================================*/

