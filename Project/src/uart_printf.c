#include "stm32f10x.h"
#include <stdio.h>
#include <usart.h>
#include "stm3210c_eval.h"
/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
	
#define PRINT_BUFFER_SIZE			256

uint8_t print_buffer[PRINT_BUFFER_SIZE];
int in = 0;
int out = 0;
int printing = 0;
	
#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */	


static void uart_print(void);	
static int uart_write(uint8_t *ptr, int32_t len);
	
uint8_t uart_send(uint8_t *p, uint32_t len)
{
		uint32_t i;
		
		
		for(i=0; i<len; i++)
		{
			while ((USART_GetFlagStatus(EVAL_COM1, USART_FLAG_TC) == RESET))
			{}
			if(printing)
				return 0;
			printing = 1;
			/* write a character to the USART */
			USART_SendData(EVAL_COM1, *p);
			printing=0;
			++p;
		}
		
		return len;
			
}	

//void uart_print(void) {


//		if (in < out) {

//			uart_send(&print_buffer[out], PRINT_BUFFER_SIZE - out);
//			out = 0;
//		}
//		else {

//			uart_send(&print_buffer[out], in - out);
//			out = in;
//		}
//}

void uart_print(void) 
{
	uint32_t i;

	if (in < out) 
	{
		for(i=0; i<((PRINT_BUFFER_SIZE - out)&&(in!=out)); i++)
		{
			if(printing==1)
				return;
			else
			{
				printing=1;
				USART_Put_Char(print_buffer[out]);
				printing=0;
				out++;
				if(out>=PRINT_BUFFER_SIZE)
					out = 0;
			}
		}
	}
	else 
	{
		for(i=0; (i<(in-out)&&(in!=out)); i++)
		{
			if(printing==1)
				return ;
			else
			{
				printing=1;
				USART_Put_Char(print_buffer[out]);
				printing=0;
				out++;					
			}
		}
	}
}
void uart_ll_print(void) {
	if (printing)
		return;
	uart_print();
}
	
static int uart_write(uint8_t *ptr, int32_t len) 
{

    int i;

  for (i = 0; i < len; i++) 
	{
    	print_buffer[in] = ptr[i];
    	in++;
    	if (in >= PRINT_BUFFER_SIZE) {
    		in = in - PRINT_BUFFER_SIZE;
    	}

    	if (in == out) {
    		return i; 
    	};
  }
    uart_ll_print();
	return len;
}	
/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
	uart_write((uint8_t*)&ch, 1);
  return ch;
}

void uart_print_tx_complete_cb(void) 
{

	uart_ll_print();
}



